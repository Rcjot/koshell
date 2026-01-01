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

int parse_strtok(char *argv[], char *line) {
  int argc = 0;
  char *token = strtok(line, " \t\n");

  while (token != NULL) {
      argv[argc++] = token;
      token = strtok(NULL, " \t\n");
  }
  argv[argc] = NULL;

  return argc;
}

int tokenizer(Token *tokens, char *line, int line_length) {
  int argc = 0;
  int i = 0;

  while (i < line_length) {
    if (is_whitespace(line[i])) {
      i++;
      continue;
    } else if (line[i] == '|') {
      Token new_token =  {TOK_PIPE, NULL};
      tokens[argc] = new_token;
      i++;
    } else if (line [i] == '<') {
      Token new_token = {TOK_REDIR_IN, NULL};
      tokens[argc] = new_token;
      i++;
    } else if (line[i] == '>') {
      Token new_token =  {TOK_REDIR_OUT, NULL};
      tokens[argc] = new_token;
      i++;
    } else if (line[i] == '>' && line[i+1] == '>') {
      Token new_token = {TOK_APPEND, NULL};
      tokens[argc] = new_token;
      i++;
    } else {
      char *token_value = malloc(128);
      int s = 0;
      while (!is_whitespace(line[i]) && !is_builtin(line[i])  && i < line_length) {
        printf("%d %d %c %d %d\n", s, i, line[i], line_length, !is_whitespace(line[i]));
        token_value[s] = line[i];
        i++;
        s++;
      }
      token_value[s] = '\0';

      if (i == line_length) {
        break;
      }
      
      Token new_token = {TOK_WORD, token_value};
      tokens[argc] = new_token;
    }
    argc++;

  }
  return argc;
}



int main () {
  
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  char *argv[MAXARGS];
  char cwd[1024];
  size_t argc = 0;
  int fd[2];
  Token *tokens;

  while (1) {
    getcwd(cwd, sizeof(cwd));
    printf("koshell:%s> ", cwd);
    fflush(stdout);
    
    int background = 0;

    n = getline(&line, &cap, stdin);
    if (n == -1) break;


    argc = tokenizer(tokens, line, n);



    if (argc == 0) continue;

    // if (strcmp(argv[argc - 1], "&") == 0) {
    //   background = 1;
    //   argv[argc-1] = NULL;
    // }

    printf("tokens: %ld \n", argc);

    for (int i = 0; i < argc; i++) {
      printf("%d %s\n", tokens[i].type, tokens[i].value);
    }

    // if (strcmp(argv[0], "cd") == 0) {
    //   const char *dir =(argv[1] != NULL) ? argv[1] : "/home";
    //   if(chdir(dir) != 0) perror("cd");
    // } else if (strcmp(argv[0], "exit") == 0) {
    //         // included user specified exit code for completeness
    //   int code = (argv[1] != NULL) ? atoi(argv[1]) : 0;
    //         // != NULL not because at default uninitialized indices in arrays are null
    //         // but it alr guarantees that the first unused index is NULL
    //   exit(code);
    // } else {
    //   pid_t pid = fork();
    //
    //   if (pid == 0) {
    //     execvp(argv[0], argv);
    //     perror("execvp");
    //     exit(1);
    //   } else {
    //     if (background) printf("[pid] %d\n", pid);
    //     else waitpid(pid, NULL, 0);
    //   }
    // }
  }
}
