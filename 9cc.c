#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

// Tokenizer

enum {
  TK_NUM = 256,
  TK_EOF,
};

// Token type
typedef struct {
  int ty; // Token type
  int val; // Number literal
  char *input; // Token string (for error reporting)
} Token;

// Tokenized input is stored to this array.
Token tokens[100];

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // Skip whitespace
    if (isspace(*p)) {
      p++;
      continue;
    }

    // + or -
    if (*p == '+' || *p == '-') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    // Number
    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "connot tokenize: %s", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
}

// Recursive-descendent parser

int pos = 0;

enum {
  ND_NUM = 256, // Number literal
};

typedef struct Node {
  int ty; // Node type
  struct Node *lhs; // left-hand side
  struct Node *rhs; // right-hand side
  int val; // Number literal
} Node;

Node *new_node(int op, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

// An error reporting function
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Node *number() {
  Node *lhs = number();
  for (;;) {
    int op = tokens[pos].ty;
    if (op != '+' && op != '-')
      break;
    pos++;
    lhs = new_node(op, lhs, number());
  }

  if (tokens[pos].ty != TK_EOF)
    error("stray token: %s", tokens[pos].input);
  return lhs;
}

// expr
// generate AST Nodes
// 一文字読んでparseする
Node *expr() {
  Node *lhs = number();
  for (;;) {
    int op = tokens[pos].ty;
    if (op != '+' && op != '-')
      break;
    pos++;
    lhs = new_node(op, lhs, number());
  }

  if (tokens[pos].ty != TK_EOF)
    error("stray token: %s", tokens[pos].input);
  return lhs;
}

// Code generator

char *regs[] = {"rdi", "rsi", "r10", "r11", "r12", "r13", "r14", "r15", NULL};
int cur;

char *gen(Node *node) {
  if (node->ty == ND_NUM) {
    char *reg = regs[cur++];
    if (!reg)
      error("register exhausted");
    printf("  mov %s, %d\n", reg, node->val);
    return reg;
  }

  char *dst = gen(node->lhs);
  char *src = gen(node->rhs);

  switch (node->ty) {
    case '+':
      printf("  add %s, %s\n", dst, src);
      return dst;
    case '-':
      printf("  sub %s, %s\n", dst, src);
      return dst;
    default:
      assert(0 && "unknown operator");
  }
}


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: 9cc <code> \n");
    return 1;
  }

  // Tokenize and parse
  tokenize(argv[1]);

  Node* node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Verify that the given expression starts with a number,
  // and then emit the first `mov` instruction
  if (tokens[0].ty != TK_NUM)
    fail(0);
  printf("  mov rax, %d\n", tokens[0].val);

  // Emit assembly as we consume the sequence of `+ <number>`
  // // or `- <number>`
  int i = 1;
  while (tokens[i].ty != TK_EOF) {
    if (tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM)
      fail(i);
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if (tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM)
      fail(i);
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }
  }

  // Generate code while descending the parse tree.
  printf("  mov rax, %s\n", gen(node));
  printf("  ret\n");
  return 0;
} 
