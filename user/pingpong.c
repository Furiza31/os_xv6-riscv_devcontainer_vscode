#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PING_PONG 5

static void usage()
{
  fprintf(2, "Usage : pingpong <number of pong>. Default is 5\n");
  exit(1);
}

static int is_number(char *s)
{
  for(int i = 0; s[i] != '\0'; i++){
    if(s[i] < '0' || s[i] > '9'){
      return 0;
    }
  }
  return 1;
}

int
main(int argc, char *argv[])
{
  int number_of_ping_pong = MAX_PING_PONG;
  int p1[2], p2[2];
  
  if(argc > 2){
    usage();
  } else if(argc == 2){
    if(!is_number(argv[1])){
      usage();
    }
    number_of_ping_pong = atoi(argv[1]);
  }

  if(pipe(p1) < 0 || pipe(p2) < 0){
    fprintf(2, "pipe error\n");
    exit(1);
  }

  int pid = fork();

  if(pid < 0){
    fprintf(2, "fork error\n");
    exit(1);
  }

  // Processus enfant
  if(pid == 0){
    close(p1[1]); // Ferme le write end de pipe p1
    close(p2[0]); // Ferme le read end de pipe p2

    for(int i = 0; i < number_of_ping_pong; i++){
      char buf;
      if(read(p1[0], &buf, 1) != 1){
        break;
      }
      fprintf(1, "<child - PID : %d>  : send pong\n", getpid());
      write(p2[1], &buf, 1);
    }

    close(p1[0]);
    close(p2[1]);
    exit(0);
  }
  // Processus parent
  else {
    close(p1[0]); // Ferme le read end de pipe p1
    close(p2[1]); // Ferme le write end de pipe p2

    for(int i = 0; i < number_of_ping_pong; i++){
      sleep(10);

      fprintf(1, "<parent - PID : %d>  : send ping\n", getpid());
      
      char buf = 'x';
      write(p1[1], &buf, 1);

      if(read(p2[0], &buf, 1) != 1){
        break;
      }
    }

    close(p1[1]);
    close(p2[0]);

    wait(0);
    exit(0);
  }
}
