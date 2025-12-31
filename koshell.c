#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAXARGS 64

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

int main () {
  
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  char *argv[MAXARGS];
  char cwd[1024];
  size_t argc = 0;

  while (1) {
    getcwd(cwd, sizeof(cwd));
    printf("koshell:%s> ", cwd);
    fflush(stdout);
    
    int background = 0;

    n = getline(&line, &cap, stdin);
    if (n == -1) break;

    argc = parse_strtok(argv, line);


    if (argv[0] == NULL) continue;

    if (strcmp(argv[argc - 1], "&") == 0) {
      background = 1;
      argv[argc-1] = NULL;
    }

    if (strcmp(argv[0], "cd") == 0) {
      const char *dir =(argv[1] != NULL) ? argv[1] : "/home";
      if(chdir(dir) != 0) perror("cd");
    } else if (strcmp(argv[0], "exit") == 0) {
            // included user specified exit code for completeness
      int code = (argv[1] != NULL) ? atoi(argv[1]) : 0;
            // != NULL not because at default uninitialized indices in arrays are null
            // but it alr guarantees that the first unused index is NULL
      exit(code);
    } else {
      pid_t pid = fork();

      if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
      } else {
        if (background) printf("[pid] %d\n", pid);
        else waitpid(pid, NULL, 0);
      }
    }
  }
}
