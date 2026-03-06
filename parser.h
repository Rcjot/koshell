#ifndef PARSER_H
#define PARSER_H


int is_whitespace(char mychar);

int is_builtin(char mychar);

int parse_strtoke(char *tokens[], char *line);

int tokenizer(Token *tokens, char *line, int line_length);


int parse_tokens(Command *commands, Token *tokens, int tokenc);


#endif 
