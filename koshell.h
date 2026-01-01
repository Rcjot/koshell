#ifndef KOSHELL_H
#define KOSHELL_H

#define MAXARGS 64

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
  char **argv;
  int in_fd;
  int out_fd;
} Command;

#endif 
