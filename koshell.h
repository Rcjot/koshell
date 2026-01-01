#ifndef KOSHELL_H
#define KOSHELL_H

#define MAXARGS 64

typedef enum {
  TOK_WORD,
  TOK_PIPE,
  TOK_REDIR_IN,
  TOK_REDIR_OUT,
  TOK_APPEND
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;



#endif 
