#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int parent_int = 10;
  char parent_msg[50] = "parent message";
  
  int p[2];
  if(pipe(p) < 0) {
    fprintf(2, "pipe creation failed\n");
    exit(1);
  }
  
  printf("--- Test du cloisonnement de données entre processus\n\n");
  printf("Parent datas : int parent_int = %d | char *parent_msg = \"%s\"\n\n", 
         parent_int, parent_msg);
  
  int pid = fork();
  if(pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }
  
  if(pid == 0) { // Child process
    close(p[0]);
    
    printf("we are in child and we have all copies of data's parent\n");
    printf("Datas from parent:\n");
    printf("parent_int : %d\n", parent_int);
    printf("parent_msg : %s\n\n", parent_msg);
    
    // Modify the values
    parent_int += 10;
    printf("child add 10 to parent_int variable. It should be 20 now\n");
    printf("from child, parent_int : %d\n", parent_int);
    
    strcpy(parent_msg, "changed by child ;)");
    printf("child change parent_msg variable. It should be \"changed by child ;)\" now from child, parent_msg : %s\n\n", parent_msg);
    
    write(p[1], "done", 4);
    close(p[1]);
    exit(0);
  } else { // Parent process
    close(p[1]);
    
    char buf[10];
    read(p[0], buf, sizeof(buf));
    close(p[0]);
    
    printf("\nparent : child exited\n");
    printf("from parent : parent_int : %d\n", parent_int);
    printf("from parent : parent_msg : %s\n\n", parent_msg);
    
    wait(0);
  }
  
  return 0;
}