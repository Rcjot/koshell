#ifndef KOSHELL_H
#define KOSHELL_H

#define MAXARGS 64
#define INIT_COMMAND_SIZE 10

typedef struct {
  int capacity;
  int size;
  char **data;
} DynArgv;

typedef enum {
  TOK_WORD,
  TOK_PIPE,
  TOK_REDIR_IN,
  TOK_REDIR_OUT,
  TOK_APPEND,
  TOK_NULL
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;

typedef struct {
  DynArgv argv;
  int in_fd;
  int out_fd;
} Command;

void push(DynArgv *arr, char *token);

#endif 
