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
  fds[0] = -1;
  fds[1] = -1;
  while (1) {
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
      perror("getcwd");
    }
    printf("koshell:%s> ", cwd);
    fflush(stdout);
    
    // int background = 0;

    n = getline(&line, &cap, stdin);
    printf("n: %ld   %s\n", n, line);
    if (n == -1) break;

    printf("line n : %ld\n", n);
  
    if (n == 1) {
      // n == 1 if user input is nothing (enter);
      continue;
    }


    tokenc = tokenizer(tokens, line, n);
    for (size_t i = 0; i < tokenc; i++) {
      printf("%d %s\n", tokens[i].type, tokens[i].value);
    }
    printf("tokens: %ld \n", tokenc);

    commandc = parse_tokens(commands,tokens, tokenc);
    printf("commandc: %ld \n", commandc);
    if (commandc < 0) {
      printf("syntax error: parsing tokens\n");
      continue;
    }


    printf("commands: %ld \n", commandc);

    


    for (ssize_t i = 0; i < commandc; i++) {
      printf("argv size : %d\n", commands[i].argv.size);
      printf("in_fd: %d, out_fd: %d\n", commands[i].in_fd, commands[i].out_fd);

    }

    if (tokenc == 0) continue;

    printf("----------------------- \n");
    int prev_in_fd = -1;

    pid_t pid;

    for (ssize_t i = 0; i < commandc; i++) {
      Command curr_command = commands[i];
      char **command_tokens = curr_command.argv.data;
      int has_pipe = (i != commandc - 1);

      // printf("%ld of %ld\n", i, commandc);

      if (has_pipe) {
        if (pipe(fds)<0) {
          perror("pipe");
          exit(1);
        }     
        printf("piped fds[0] %d, fds[1] %d\n", fds[0], fds[1]);
      } else {
        fds[0] = -1;
        fds[1] = -1;
      }


      pid = fork();
      printf("forking %d\n", pid);

      if (pid == 0) {
        printf("     %s\n", command_tokens[0]);
        printf("     child with in_fd %d out_fd %d with prev_in_fd %d\n", curr_command.in_fd, curr_command.out_fd, prev_in_fd);
        printf("     fds[0] %d, fds[1] %d\n", fds[0], fds[1]);

        printf("     > child closing fds[0] : %d\n", fds[0]);
        close(fds[0]);

        if (curr_command.in_fd > 0) {
          dup2(prev_in_fd, 0);
          printf("     reading from %d\n", prev_in_fd);
          close(prev_in_fd);
        }        

        if (curr_command.out_fd > 0) {
          printf("     writing to %d\n", fds[1]);
          dup2(fds[1], 1);
          close(fds[1]);
        } else {
          close(fds[1]);
        }

        execvp(command_tokens[0], command_tokens);
        perror("execvp");
        exit(1);
      } else {
        printf("parent\n");
        printf("     command with in_fd:  %d, out_fd:  %d\n", curr_command.in_fd, curr_command.out_fd);
        printf("        with prev_in_fd %d\n", prev_in_fd);
        printf("     fds[0] %d, fds[1] %d\n", fds[0], fds[1]);

        printf("     > closing previous reader: %d\n", prev_in_fd);
        close(prev_in_fd);
        
        if (has_pipe) {
          prev_in_fd = fds[0];
          printf("     assigned prev_in_fd = %d\n",  fds[0]);
          close(fds[1]);
          printf("     > has pipe: closing writer: %d\n", fds[1]);
        } else {
          prev_in_fd = -1;
          if (curr_command.out_fd < 0) {
            // might be unnecessary since out_fd < 0 means no pipe fds[1]
            printf("     > no pipe: closing writer: %d\n", fds[1]);
            close(fds[1]);
          }  
        }
      }
    }
    while (wait(NULL) > 0); 
  }
}
