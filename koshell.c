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

  while (i < line_length) {
    // printf("%c is the character at %d\n", line[i], i);
    if (is_whitespace(line[i])) {
      i++;
      continue;
    } else if (line[i] == '|') {
      Token new_token =  {TOK_PIPE, NULL};
      tokens[tokenc] = new_token;
      i++;
    }  else if (line[i] == '>' && line[i+1] == '>') {
      Token new_token = {TOK_APPEND, NULL};
      tokens[tokenc] = new_token;
      i++;
      i++;
    } else if (line [i] == '<') {
      Token new_token = {TOK_REDIR_IN, NULL};
      tokens[tokenc] = new_token;
      i++;
    } else if (line[i] == '>') {
      Token new_token =  {TOK_REDIR_OUT, NULL};
      tokens[tokenc] = new_token;
      i++;
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
    }
    // for convention and clarity, token after last should be null
    tokenc++;

  }

  // printf("passed last tokenc is %ld\n", tokenc);
  Token null_token = {TOK_NULL, NULL};
  tokens[tokenc++] = null_token;
  // for convention and clarity, to mark end of token list should be null

  return tokenc;
}

int parse_tokens(Command *commands, Token *tokens, int tokenc, int fds[]) {
  int commandc = 0;
  commands[commandc].argv.size = 0;
  commands[commandc].argv.capacity = 10;
  commands[commandc].argv.data = malloc(sizeof(char *) * 10);

  // initialize in_fd and out_fd
  commands[commandc].in_fd = -1;
  commands[commandc].out_fd = -1;

  printf("fds: %d %d\n", fds[0], fds[1]);

  for (int i = 0; i < tokenc; i++) {
    // printf("seeing %s at index %d with type %d\n", tokens[i].value, i, tokens[i].type);
  

    switch(tokens[i].type) {
      case TOK_WORD :
        
        push(&commands[commandc].argv, tokens[i].value);
        // printf(" %s pushed\n", tokens[i].value);

        break;
      case TOK_PIPE :
        if (i > 0) {
          printf("token before : %d\n", tokens[i - 1].type);
        }
        if (i == 0) return -1;
        if ( tokens[i-1].type >0) return -1;

        commands[commandc].out_fd = fds[1];
        commands[commandc].in_fd = -1;


        push(&commands[commandc].argv, NULL);

        commandc++;

        commands[commandc].out_fd = -1;
        commands[commandc].in_fd = fds[0];

        commands[commandc].argv.data = malloc(sizeof(char *) * 10);
        commands[commandc].argv.size = 0;
        commands[commandc].argv.capacity = 10;

        break;
      case TOK_REDIR_IN :
        break;
      case TOK_REDIR_OUT :
        break;
      case TOK_APPEND :
        break;
      case TOK_NULL :
        push(&commands[commandc].argv, NULL);
        commandc++;
        break;
    }

  }

  return commandc;


}



int main () {
  
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  char cwd[1024];
  size_t tokenc = 0;
  ssize_t commandc = 0;
  Token tokens[MAXARGS];
  Command *commands = malloc(sizeof(Command) * INIT_COMMAND_SIZE);
  int fds[2];
  if(pipe(fds) < 0) {
    perror("pipe");
  }

  while (1) {
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
      perror("getcwd");
    }
    printf("koshell:%s> ", cwd);
    fflush(stdout);
    
    int background = 0;

    n = getline(&line, &cap, stdin);
    if (n == -1) break;

  
    if (n == 1) {
      // n == 1 if user input is nothing (enter);
      continue;
    }


    tokenc = tokenizer(tokens, line, n);

    commandc = parse_tokens(commands,tokens, tokenc, fds);
    if (commandc < 0) {
      printf("syntax error: parsing tokens\n");
      continue;
    }


    printf("tokens: %ld \n", tokenc);
    printf("commands: %ld \n", commandc);

    for (size_t i = 0; i < tokenc; i++) {
      printf("%d %s\n", tokens[i].type, tokens[i].value);
    }


    for (ssize_t i = 0; i < commandc; i++) {
      printf("argv size : %d\n", commands[i].argv.size);
      printf("in_fd: %d, out_fd: %d\n", commands[i].in_fd, commands[i].out_fd);

    }







    if (tokenc == 0) continue;

    // tokenc -1 -1 because we want to look at second to the last token, since last is null token
    // tokenc is guranteed to atleast have length 2
    if (strcmp(tokens[tokenc - 1 -1].value, "&") == 0) {
      background = 1;
      Token null_token = {TOK_NULL, NULL};
      tokens[tokenc-1] = null_token;
    }

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
        // printf("child performing\n");
        char *argv[MAXARGS];
        int tokenc_notnull = 0;
        for (size_t i = 0; i < tokenc - 1; i++) {
          // printf("%s %d\n", tokens[i].value, i);
          argv[tokenc_notnull] = tokens[i].value;
          tokenc_notnull++;
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
