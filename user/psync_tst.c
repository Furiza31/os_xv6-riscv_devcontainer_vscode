#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int wait_flag = 1; // Default wait for child

  if(argc > 2){
    fprintf(2, "Usage:\n\ttpsync_tst\n\tpsync_tst [1 or 2]\n\t0: do not wait for child, 1: wait for child\n");
    exit(1);
  }

  // If a parameter is provided, check its value.
  if(argc == 2) {
    int param = atoi(argv[1]);
    if(param != 1 && param != 0){
      fprintf(2, "Usage:\n\ttpsync_tst\n\tpsync_tst [1 or 0]\n\t0: do not wait for child, 1: wait for child\n");
      exit(1);
    }
    wait_flag = (param == 1);
  }

  printf("-- Test process synchronisation --\n");

  int pid = fork();

  if(pid < 0){
    fprintf(2, "Humm, something went wrong. Fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // Child process
    printf("Hi, I am the child process with pid %d\n", getpid());
    printf("I will sleep for 2 seconds\n");
    sleep(2);
    printf("I end my life now\n");
    exit(0);
  } else {
    // Parent process
    if(wait_flag) {
      int child_pid = wait(0);
      if(child_pid < 0) {
        fprintf(2, "Humm, something went wrong. The parent has no child process.\n");
        exit(1);
      }
    }
    printf("Hi, I am the parent process with pid %d\n", getpid());
    if(wait_flag)
      printf("I was waiting for my child process to finish.\n");
    else
      printf("I did not wait for my child process.\n");
  }
  exit(0);
}
