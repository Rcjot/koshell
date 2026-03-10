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

  // do not include null terminator in parsing line
  while (i < line_length - 1) {
    if (is_whitespace(line[i])) {
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
      // IF TOK_WORD
    

      char *token_value = malloc(128);
      // i did not handle anything for tokens that exceed 128 characters
      int s = 0;
      // iterator for token_value

      while (!is_whitespace(line[i]) && !is_builtin(line[i])  && i < line_length) {
        token_value[s] = line[i];
        i++;
        s++;
      }
      if (token_value[s-1] != '\0') {
        // to handle token to always have \0, two cases: token is between tokens(builtin) or token is last token
        token_value[s] = '\0';
      }
      
      Token new_token = {TOK_WORD, token_value};
      tokens[tokenc] = new_token;
      tokenc++;
    }
  }

  Token null_token = {TOK_NULL, NULL};
  tokens[tokenc++] = null_token;
  // for convention and clarity, to mark end of token list should be null

  return tokenc;
}

int parse_tokens(Command *commands, Token *tokens, int tokenc) {
  int commandc = 0;
  commands[commandc].argv.size = 0;
  commands[commandc].argv.capacity = 10;

  // allocate only 10 tokens in an argv
  commands[commandc].argv.data = malloc(sizeof(char *) * 10);


  // initialize in_fd and out_fd
  commands[commandc].in_fd = -1;
  commands[commandc].out_fd = -1;

  // printf("tokenc: %d\n", tokenc);

  for (int i = 0; i < tokenc; i++) {
    // ITERATE tokens


    switch(tokens[i].type) {
      case TOK_WORD :{        
        // printf("token %d : %s\n", i,  tokens[i].value);
        push(&commands[commandc].argv, tokens[i].value);
        // push is a defined helper function for dynarr

        break;
      }
      case TOK_PIPE : {
        
        // if i == 0 means its the first token of the line
        if (i == 0) return -1;

        if ( tokens[i+1].type > 0) return -1;

        commands[commandc].out_fd = 1;
        if (i == 0) {
          commands[commandc].in_fd = -1;
        }


        push(&commands[commandc].argv, NULL);

        // increment next command
        commandc++;

        commands[commandc].out_fd = -1;
        commands[commandc].in_fd = 1;

        // allocate only 10 tokens in an argv?
        commands[commandc].argv.data = malloc(sizeof(char *) * 10);
        commands[commandc].argv.size = 0;
        commands[commandc].argv.capacity = 10;

        break;

      }
      case TOK_REDIR_IN : {// <
        if (i == 0) return -1;
        // means if next token is not TOK_WORD
        if (tokens[i+1].type > 0) return -1;

        if (commands[commandc].in_fd > 1) {
          // this avoids fd leaks ex. wc < file1 < file2
          close(commands[commandc].in_fd);
        }

        // next token should be TOK_WORD and is a file
        // we don't include it in the commands argv, thats why ++i
        int next_tok = ++i;

        int open_fd = open(tokens[next_tok].value, O_RDONLY);
        free(tokens[next_tok].value);
        // we free since we wont keep this TOK_WORD token
        // freeing for other tokens is done later in the commands

        if (open_fd < 0) {
          // if input file doesnt exist
          // we free commands here because commandc is not returned to main
          // commandc + 1, since it hasn't incremented yet
          free_commands(commands, commandc + 1);
          perror("cant find directory");
          return -1;
        }
        commands[commandc].in_fd = open_fd;

        break;
      }
      case TOK_REDIR_OUT : { // >
        if (i == 0) return -1;
        if ( tokens[i+1].type > 0) return -1;

        if (commands[commandc].out_fd > 1) {
          close(commands[commandc].out_fd);
        }

        // next token should be TOK_WORD and is a file
        // we don't include it in the commands argv, thats why ++i
        int next_tok = ++i;

        int open_fd = open(tokens[next_tok].value, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
        free(tokens[next_tok].value);

        commands[commandc].out_fd = open_fd;

        break;
      }
      case TOK_APPEND : {// >>
        if (i == 0) return -1;
        if ( tokens[i+1].type > 0) return -1;

        if (commands[commandc].out_fd > 1) {
          close(commands[commandc].out_fd);
        }

        // next token should be TOK_WORD and is a file
        // we don't include it in the commands argv, thats why ++i
        int next_tok = ++i;

        int open_fd = open(tokens[next_tok].value, O_WRONLY | O_CREAT | O_APPEND, 0644); 
        free(tokens[next_tok].value);

        commands[commandc].out_fd = open_fd;



        break;
      }
      case TOK_NULL : {
        push(&commands[commandc].argv, NULL);
        commandc++;
        break;
      }
    }

  }

  return commandc;


}


