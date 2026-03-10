#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "koshell.h"
#include "parser.h"



int main () {
  
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  char cwd[1024];
  size_t tokenc = 0;
  ssize_t commandc = 0;
  Token tokens[MAXARGS];
  Command *commands = malloc(sizeof(Command) * INIT_COMMAND_SIZE);

  // pipe
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
    // printf("n: %ld   %s\n", n, line);
    if (n == -1) break;

    // printf("line n : %ld\n", n);
  
    if (n == 1) {
      // n == 1 if user input is nothing (enter);
      continue;
    }


    tokenc = tokenizer(tokens, line, n);
    // for (size_t i = 0; i < tokenc; i++) {
    //   printf("%d %s\n", tokens[i].type, tokens[i].value);
    // }
    // printf("tokens: %ld \n", tokenc);

    commandc = parse_tokens(commands,tokens, tokenc);
    // printf("commandc: %ld \n", commandc);
    if (commandc < 0) {
      printf("syntax error: parsing tokens\n");
      continue;
    }

    // for (ssize_t i = 0; i < commandc; i++) {
    //   printf("argv size : %d\n", commands[i].argv.size)k
    //   printf("in_fd: %d, out_fd: %d\n", commands[i].in_fd, commands[i].out_fd);
    //
    // }

    if (tokenc == 0) continue;

    int prev_in_fd = -1;

    pid_t pid;

    for (ssize_t i = 0; i < commandc; i++) {
      Command curr_command = commands[i];
      char **command_tokens = curr_command.argv.data;
      int has_pipe = (i != commandc - 1);


      if (has_pipe) { 
        // initiate pipes
        if (pipe(fds)<0) {
          perror("pipe");
          exit(1);
        }     
      } else {
        fds[0] = -1;
        fds[1] = -1;
      }


      pid = fork();

      if (pid == 0) {
        // child
        close(fds[0]);

        if (curr_command.in_fd > 0) {
          // change stdin to prev_in_fd
          dup2(prev_in_fd, 0);
          // remember fds are just ints that point to a buffer
          // dup2 just made new reference stdin pointing to that buffer, we can then close prev_in_fd as its reference
          // only this fd in this process is closed
          close(prev_in_fd);
        }        

        if (curr_command.out_fd > 0) {
          // change stdout to fds[1] write end of pipe
          dup2(fds[1], 1);
        }

        close(fds[1]);

        execvp(command_tokens[0], command_tokens);
        perror("execvp");
        exit(1);
      } else {
        // parent
        close(prev_in_fd);
        
        if (has_pipe) {
          // assign prev_in_fd to pipe read
          prev_in_fd = fds[0];

          close(fds[1]);
        } else {
          prev_in_fd = -1;
        }
      }
    }
    while (wait(NULL) > 0); 
  }
}
