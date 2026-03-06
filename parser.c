#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "koshell.h"
#include "parser.h"

int is_whitespace(char mychar) {
  if (mychar == '\n' || mychar == ' ' || mychar == '\t') return 1;
  else return 0;
}

int is_builtin(char mychar) {
  if (mychar == '|' || mychar == '<' || mychar == '>') return 1;
  else return 0;
}

int parse_strtok(char *tokens[], char *line) {
  size_t tokenc = 0;
  char *token = strtok(line, " \t\n");

  while (token != NULL) {
      tokens[tokenc++] = token;
      token = strtok(NULL, " \t\n");
  }
  tokens[tokenc] = NULL;

  return tokenc;
}

int tokenizer(Token *tokens, char *line, int line_length) {
  line[line_length - 1] = '\0';
  // bcus i think getline doesnt automatically append '\0' but '\n'
  size_t tokenc = 0;
  int i = 0;
  // int bool = is_builtin('\0');
  // printf("%d linelgnth: %d\n", line_length, bool);

  // do not include null terminator in parsing line
  while (i < line_length - 1) {
    // printf("%c is the character at %d\n", line[i], i);
    if (is_whitespace(line[i])) {
      // printf("        %c is the character at %d is whitespace\n", line[i], i);
      i++;
      continue;
    } else if (line[i] == '|') {
      Token new_token =  {TOK_PIPE, NULL};
      tokens[tokenc] = new_token;
      i++;
      tokenc++;
    }  else if (line[i] == '>' && line[i+1] == '>') {
      Token new_token = {TOK_APPEND, NULL};
      tokens[tokenc] = new_token;
      i++;
      i++;
      tokenc++;
    } else if (line [i] == '<') {
      Token new_token = {TOK_REDIR_IN, NULL};
      tokens[tokenc] = new_token;
      i++;
      tokenc++;
    } else if (line[i] == '>') {
      Token new_token =  {TOK_REDIR_OUT, NULL};
      tokens[tokenc] = new_token;
      i++;
      tokenc++;
    } else {
      char *token_value = malloc(128);
      // i did not handle anything for tokens that exceed 128 characters
      int s = 0;

      while (!is_whitespace(line[i]) && !is_builtin(line[i])  && i < line_length) {
        // printf("%d %d %c %d %d\n", s, i, line[i], line_length, !is_whitespace(line[i]));
        token_value[s] = line[i];
        i++;
        s++;
      }
      if (token_value[s-1] != '\0') {
        // to handle token to always have \0, two cases: token is between tokens or token is last token
        token_value[s] = '\0';
      }
      
      Token new_token = {TOK_WORD, token_value};
      tokens[tokenc] = new_token;
      tokenc++;
    }
  }

  // printf("passed last tokenc is %ld\n", tokenc);
  Token null_token = {TOK_NULL, NULL};
  tokens[tokenc++] = null_token;
  // for convention and clarity, to mark end of token list should be null

  return tokenc;
}

int parse_tokens(Command *commands, Token *tokens, int tokenc) {
  int commandc = 0;
  commands[commandc].argv.size = 0;
  commands[commandc].argv.capacity = 10;
  commands[commandc].argv.data = malloc(sizeof(char *) * 10);

  // initialize in_fd and out_fd
  commands[commandc].in_fd = -1;
  commands[commandc].out_fd = -1;

  printf("tokenc: %d\n", tokenc);

  // printf("fds: %d %d\n", fds[0], fds[1]);

  for (int i = 0; i < tokenc; i++) {
    // printf("seeing %s at index %d with type %d\n", tokens[i].value, i, tokens[i].type);
  

    switch(tokens[i].type) {
      case TOK_WORD :
        
        printf("token %d : %s\n", i,  tokens[i].value);
        push(&commands[commandc].argv, tokens[i].value);
        // printf(" %s pushed\n", tokens[i].value);

        break;
      case TOK_PIPE :
        printf("current idx : %d\n", i);
        // if (i > 0) {
        //   printf("token before : %d %s\n", tokens[i + 1].type, tokens[i + 1].value);
        // }
        if (i == 0) return -1;
        printf("token before : %d %s\n", tokens[i + 1].type, tokens[i + 1].value);
        if ( tokens[i+1].type > 0) return -1;

        commands[commandc].out_fd = 1;
        if (i == 0) {
          commands[commandc].in_fd = -1;
        }


        push(&commands[commandc].argv, NULL);

        commandc++;

        commands[commandc].out_fd = -1;
        commands[commandc].in_fd = 1;

        commands[commandc].argv.data = malloc(sizeof(char *) * 10);
        commands[commandc].argv.size = 0;
        commands[commandc].argv.capacity = 10;

        break;
      case TOK_REDIR_IN :
        if (i == 0) return -1;
        if ( tokens[i+1].type > 0) return -1;
        break;
      case TOK_REDIR_OUT :
        if (i == 0) return -1;
        if ( tokens[i+1].type > 0) return -1;
        break;
      case TOK_APPEND :
        if (i == 0) return -1;
        if ( tokens[i+1].type > 0) return -1;
        break;
      case TOK_NULL :
        push(&commands[commandc].argv, NULL);
        commandc++;
        break;
    }

  }

  return commandc;


}


