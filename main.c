#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef enum TYPE {
  NUMBER,
  OPERATOR
} TYPE;

typedef struct Token {
  TYPE type;
  union {
    double value;
    char operator;
  };
} Token;

typedef struct c_list {
  void* val;
  struct c_list *next;
} c_list;

c_list* listInit(void* val) {
  c_list *new = (c_list*) malloc(sizeof(c_list));
  if (!new) {
    fprintf(stderr, "allocation failed!");
    exit(EXIT_FAILURE);
  }
  new->val = val;
  new->next = NULL;
  return new;
}

typedef struct c_stack {
  c_list *head;
} c_stack;

c_stack* stackInit() {
  c_stack* new = (c_stack*) malloc(sizeof(c_stack));
  new->head = NULL;
  return new;
}

int stackIsEmpty(c_stack *stack) {
  return stack->head == NULL;
}

int stackSize(c_stack *stack) {
  int i = 0;
  for (c_list *cur = stack->head; cur; cur = cur->next) {
    i++;
  }
  return i;
}

void stackPush(c_stack *stack, void* val) {
  c_list *new = listInit(val);

  new->next = stack->head;
  stack->head = new;
}

void stackPop(c_stack *stack) {
  if (stackIsEmpty(stack)) {
    fprintf(stderr, "Underflow!\n");
    return;
  }

  c_list *temp = stack->head;
  stack->head = stack->head->next;
  free(temp);
}

void* stackPeek(c_stack *stack) {
  if (stackIsEmpty(stack)) {
    printf("Empty!");
    return (void*) -1;
  }

  return stack->head->val;
}

typedef struct c_queue {
  c_list *front, *back;
} c_queue;

c_queue* queueInit() {
  c_queue *new = (c_queue*) malloc(sizeof(c_queue));
  new->front = NULL;
  new->back = NULL;
  return new;
}

int queueIsEmpty(c_queue *queue) {
  return (queue->front == queue->back && queue->front == NULL);
}

void enqueue(c_queue *queue, void *val) {
  c_list *new = listInit(val);
  if (queue->back == NULL) {
    queue->front = queue->back = new;
    return;
  }

  queue->back->next = new;
  queue->back = new;
}

void deque(c_queue *queue) {
  if (queueIsEmpty(queue)) {
    fprintf(stderr, "Queue Underflow\n");
    return;
  }

  c_list *temp = queue->front;
  queue->front = queue->front->next;

  if (queue->front == NULL)
    queue->back = NULL;

  free(temp);
}

void* queuePeek(c_queue* queue)
{
    if (queueIsEmpty(queue)) {
        printf("Queue is empty\n");
        return (void*)-1;
    }
    return queue->front->val;
}

int isOperator(char c) {
  return (c == '+' || c == '-' || c == '*' || c == '/' || c =='^' || c == '(' || c == ')');
}

int operatorPrec(char operator) {
  int precedence;
  switch (operator) {
    case '^':
      precedence = 3;
      break;
    case '*':
      precedence = 2;
      break;
    case '/':
      precedence = 2;
      break;
    case '+':
      precedence = 1;
      break;
    case '-':
      precedence = 1;
      break;
  }
  return precedence;
}

double operate(double num1, double num2, char op) {
  double res;
  switch (op) {
    case '^':
      res = pow(num1, num2);
      break;
    case '*':
      res = num1*num2;
      break;
    case '/':
      res = num1/num2;
      break;
    case '+':
      res = num1 + num2;
      break;
    case '-':
      res = num1 - num2;
      break;
  }
  return res;
};

c_queue* tokenize(char* expr) {
  c_queue *tokens = queueInit();
  char *c = expr;
  while (*c) {
    Token *token = (Token*) malloc(sizeof(Token));
    if (*c == ' ') {
      c++;
      continue;
    }
    if ('0' <= *c && *c <= '9') {
      token->type = NUMBER;
      token->value = 0;
      while ('0' <= *c && *c <= '9') {
        token->value = token->value*10 + (*c - '0');
        c++;
      }
      enqueue(tokens, token);
      continue;
    }
    if (isOperator(*c)) {
      token->type = OPERATOR;
      token->operator = *c;
      enqueue(tokens, token);
      c++;
      continue;
    }
    fprintf(stderr, "Invalid input: %c\n", *c);
    exit(EXIT_FAILURE);
  }

  return tokens;
}

double calc(char* expr) {
  c_stack *operators = stackInit(), *res = stackInit();
  c_queue *rpn = queueInit(), *tokens;
  tokens = tokenize(expr);

  while (!queueIsEmpty(tokens)) {
    Token *cur = (Token*) queuePeek(tokens);
    deque(tokens);
    if (cur->type == NUMBER) {
      enqueue(rpn, cur);
    } else {
      Token *topOperator;
      if (cur->operator == '(') {
        stackPush(operators, cur);
      } else if (cur->operator == ')') {
        while (!stackIsEmpty(operators) && (topOperator = (Token*)stackPeek(operators))->operator != '(') {
          stackPop(operators);
          enqueue(rpn, topOperator);
        }
        stackPop(operators);
        continue;
      } else if (stackIsEmpty(operators)
          || operatorPrec(cur->operator) >= operatorPrec((topOperator = (Token*)stackPeek(operators))->operator)) {
        stackPush(operators, cur);
      } else {
        while (!stackIsEmpty(operators) && operatorPrec(cur->operator) <= operatorPrec(topOperator->operator)) {
          stackPop(operators);
          enqueue(rpn, topOperator);
          if (!stackIsEmpty(operators)) {
            topOperator = (Token*) stackPeek(operators);
          }
        }
        stackPush(operators, cur);
      }
      topOperator = (Token*) stackPeek(operators);
    }
  }

  while (!stackIsEmpty(operators)) {
    Token *cur = (Token*) stackPeek(operators);
    stackPop(operators);
    enqueue(rpn, cur);
  }

  while (!queueIsEmpty(rpn)) {
    Token *cur = queuePeek(rpn);
    deque(rpn);
    if (cur->type == NUMBER) {
      stackPush(res, cur);
    } else {
      Token *num2 = stackPeek(res); stackPop(res);
      Token *num1 = stackPeek(res); stackPop(res);
      Token *answer = (Token*) malloc(sizeof(Token));
      answer->value =  operate(num1->value, num2->value, cur->operator);
      stackPush(res, answer);
    }
  }

  if (stackIsEmpty(res)) {
    fprintf(stderr, "Did not evaluate correctly.\n");
    exit(EXIT_FAILURE);
  }

  double result = ((Token*)stackPeek(res))->value;
  return result;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: ./calc [expression]\n");
    return 0;
  }
  char *expression = argv[1];
  printf("%f\n", calc(expression));
  return 0;
}
