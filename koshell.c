#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "koshell.h"

int is_whitespace(char mychar) {
  if (mychar == '\n' || mychar == ' ' || mychar == '\t') return 1;
  else return 0;
}

int is_builtin(char mychar) {
  if (mychar == '|' || mychar == '<' || mychar == '>') return 1;
  else return 0;
}

int parse_strtok(char *tokens[], char *line) {
  int tokenc = 0;
  char *token = strtok(line, " \t\n");

  while (token != NULL) {
      tokens[tokenc++] = token;
      token = strtok(NULL, " \t\n");
  }
  tokens[tokenc] = NULL;

  return tokenc;
}

int tokenizer(Token *tokens, char *line, int line_length) {
  int tokenc = 0;
  int i = 0;

  while (i < line_length) {
    if (is_whitespace(line[i])) {
      i++;
      continue;
    } else if (line[i] == '|') {
      Token new_token =  {TOK_PIPE, NULL};
      tokens[tokenc] = new_token;
      i++;
    } else if (line [i] == '<') {
      Token new_token = {TOK_REDIR_IN, NULL};
      tokens[tokenc] = new_token;
      i++;
    } else if (line[i] == '>') {
      Token new_token =  {TOK_REDIR_OUT, NULL};
      tokens[tokenc] = new_token;
      i++;
    } else if (line[i] == '>' && line[i+1] == '>') {
      Token new_token = {TOK_APPEND, NULL};
      tokens[tokenc] = new_token;
      i++;
    } else {
      char *token_value = malloc(128);
      int s = 0;
      while (!is_whitespace(line[i]) && !is_builtin(line[i])  && i < line_length) {
        // printf("%d %d %c %d %d\n", s, i, line[i], line_length, !is_whitespace(line[i]));
        token_value[s] = line[i];
        i++;
        s++;
      }
      token_value[s] = '\0';

      if (i == line_length) {
        break;
      }
      
      Token new_token = {TOK_WORD, token_value};
      tokens[tokenc] = new_token;
    }
    Token null_token = {TOK_NULL, NULL};
    tokens[tokenc + 1] = null_token;
    // for convention and clarity, token after last should be null
    tokenc++;

  }
  return tokenc;
}

int parse_tokens(Command *commands, Token *tokens) {


}



int main () {
  
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  char cwd[1024];
  size_t tokenc = 0;
  size_t commandsc = 0;
  int fd[2];
  Token tokens[MAXARGS];
  Command *commands;

  while (1) {
    getcwd(cwd, sizeof(cwd));
    printf("koshell:%s> ", cwd);
    fflush(stdout);
    
    int background = 0;

    n = getline(&line, &cap, stdin);
    if (n == -1) break;


    tokenc = tokenizer(tokens, line, n);



    if (tokenc == 0) continue;

    if (strcmp(tokens[tokenc - 1].value, "&") == 0) {
      background = 1;
      Token null_token = {TOK_NULL, NULL};
      tokens[tokenc-1] = null_token;
    }

    printf("tokens: %ld \n", tokenc);

    // for (int i = 0; i < tokenc; i++) {
    //   printf("%d %s\n", tokens[i].type, tokens[i].value);
    // }

    if (strcmp(tokens[0].value, "cd") == 0) {
      const char *dir =(tokens[1].type == TOK_WORD) ? tokens[1].value : "/home";
      if(chdir(dir) != 0) perror("cd");
    } else if (strcmp(tokens[0].value, "exit") == 0) {
            // included user specified exit code for completeness
      int code = (tokens[1].type == TOK_WORD) ? atoi(tokens[1].value) : 0;
            // != NULL not because at default uninitialized indices in arrays are null
            // but it alr guarantees that the first unused index is NULL
      exit(code);
    } else {
      pid_t pid = fork();

      if (pid == 0) {
        char *argv[MAXARGS];
        for (int i = 0; i < tokenc; i++) {
          argv[i] = tokens[i].value;
        }
        // for (int i = 0; i < tokenc; i++) {
        //   printf("%s\n", argv[i]);
        // }
        execvp(tokens[0].value, argv);
        perror("execvp");
        exit(1);
      } else {
        if (background) printf("[pid] %d\n", pid);
        else waitpid(pid, NULL, 0);
      }
    }
  }
}
