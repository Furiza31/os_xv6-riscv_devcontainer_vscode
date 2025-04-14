#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/**
  * run pstree command in child process
  * and wait for it to finish
  * @param nchilds number of child processes to create
  * @return void
*/
void test_child_processes(int nchilds) {
  printf("---Test process relationship with %d child\n\n", nchilds);
  
  for (int i = 0; i < nchilds; i++) {
    int pid = fork();
    if (pid < 0) {
      fprintf(2, "Error: fork failed\n");
      exit(1);
    } else if (pid == 0) {
      printf("child %d created with PID %d\n\n", i, getpid());
      printf("child %d execute pstree and die\n", i);
      
      char *args[] = {"pstree", 0};
      exec(args[0], args);
      fprintf(2, "exec pstree failed\n");
      exit(1);
    }
    
    wait(0);
  }
}
/**
  * test genealogy of processes
  * @param gen number of generations
  * @param max_gen maximum number of generations
  * @param current_gen current generation
  * @return void
*/
void test_genealogy(int gen, int max_gen, int current_gen) {
  if (current_gen >= max_gen) {
    return;
  }
  
  printf("---Test process genealogy on %d's'th generation\n", current_gen);
  
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "Error: fork failed\n");
    exit(1);
  } else if (pid == 0) {
    printf("child %d created with PID %d from parent PID %d\n\n", current_gen, getpid(), getppid());

    // If we're at the last generation, show the complete hierarchy
    if (current_gen == max_gen - 1) {
      printf("\nLet's see process hierarchy with pstree:\n");
      int ps_pid = fork();
      if (ps_pid < 0) {
        fprintf(2, "Error: fork failed\n");
        exit(1);
      } else if (ps_pid == 0) {
        char *args[] = {"pstree", 0};
        exec(args[0], args);
        fprintf(2, "exec pstree failed\n");
        exit(1);
      }
      wait(0);
    }

    test_genealogy(gen, max_gen, current_gen + 1);
    exit(0);
  }

  wait(0);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
      fprintf(2, "Usage: phierarchy <number of childs> <number of generation>\n");
      exit(1);
  }

  int nchilds = atoi(argv[1]);
  int ngen = atoi(argv[2]);
  if (nchilds < 1 || ngen < 1) {
      fprintf(2, "Error: number of childs and generation must be greater than 0\n");
      exit(1);
  }

  test_child_processes(nchilds);
  printf("All child must be terminated : see that with pstree\n");

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "Error: fork failed\n");
    exit(1);
  } else if (pid == 0) {
    char *args[] = {"ps", 0};
    exec(args[0], args);
    fprintf(2, "exec ps failed\n");
    exit(1);
  }

  printf("\n");

  wait(0);
  test_genealogy(nchilds, ngen, 0);
  
  wait(0);
  exit(0);
}

