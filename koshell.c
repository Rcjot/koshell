#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAXARGS 64

void parse_strtok(char *argv[], char *line) {
  int argc = 0;
  char *token = strtok(line, " \t\n");

  while (token != NULL) {
      argv[argc++] = token;
      token = strtok(NULL, " \t\n");
  }
  argv[argc] = NULL;
}

int main () {
  
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  char *argv[MAXARGS];

  while (1) {
    printf("koshell> ");
    fflush(stdout);
    
    n = getline(&line, &cap, stdin);
    if (n == -1) break;

    parse_strtok(argv, line);

    pid_t pid = fork();

    if (pid == 0) {
      execvp(argv[0], argv);
      perror("execvp");
      exit(1);
    } else {
      waitpid(pid, NULL, 0);
    }

  }
}
