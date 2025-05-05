#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_NUMBER 100
#define MIN_NUMBER 1

static void usage() {
  fprintf(2, "Usage: devine <number of tries (> 0)>\n");
  exit(1);
}

static void generate_random_number(int *number) {
  uint32 r, seed = uptime();
    srand(seed);
    r = rand();
    *number = r % (MAX_NUMBER - MIN_NUMBER + 1) + MIN_NUMBER;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    usage();
  }
  int tries = atoi(argv[1]);
  if (tries <= 0) {
    usage();
  }

  int max_tries = tries;

  int p1[2]; // fils -> père (guess)
  int p2[2]; // père -> fils (number)

  if (pipe(p1) < 0 || pipe(p2) < 0) {
    fprintf(2, "pipe error\n");
    exit(1);
  }
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork error\n");
    exit(1);
  }

  tries = 0;

  if (pid != 0) {
    // Processus père
    close(p1[1]); // Ferme l'écriture
    close(p2[0]); // Ferme la lecture
    int mystery ;
    generate_random_number(&mystery );

    printf("<parent - PID : %d> : Game start - mystery number between 1 to 100 : %d\n",
      getpid(), mystery);

    int guess, feedback;
    
    while(tries < max_tries) {
      // lire le nombre proposé par le fils
      if (read(p1[0], &guess, sizeof(guess)) != sizeof(guess)) {
        fprintf(2, "read error\n");
        exit(1);
      }
      tries++;
      if (guess > mystery) {
        feedback = 1; // trop grand
        fprintf(1, "<parent - PID : %d> : number given %d higher than mystery %d\n",
                       getpid(), guess, mystery);
      } else if (guess < mystery) {
        feedback = -1; // trop petit
        fprintf(1, "<parent - PID : %d> : number given %d lesser than mystery %d\n",
                       getpid(), guess, mystery);
      } else {
        feedback = 0; // gagné
        fprintf(1, "<parent - PID : %d> : number found %d == %d in %d %s!!!\n",
          getpid(), guess, mystery, tries,
          (tries > 1 ? "tries" : "try"));
      }

      // envoyer le feedback au fils
      if (write(p2[1], &feedback, sizeof(feedback)) != sizeof(feedback)) {
        fprintf(2, "write error\n");
        exit(1);
      }
      if (feedback == 0) break;
    }

    if (tries == max_tries) {
      fprintf(1, "<parent - PID : %d> : number not found in %d tries\n",
        getpid(), max_tries);
    }

    close(p1[0]);
    close(p2[1]);
    wait(0);
  } else {
    // Processus fils
    close(p1[0]);
    close(p2[1]);
    int guess, feedback;
    int min = MIN_NUMBER, max = MAX_NUMBER;
    while (tries < max_tries) {
      guess = (min + max) / 2;
      sleep(10);
      tries++;
      printf("<child - PID : %d> : send number (0 to 100) to parent --> %d\n",
        getpid(), guess);
      
      // envoyer le nombre proposé au père
      if (write(p1[1], &guess, sizeof(guess)) != sizeof(guess)) {
        fprintf(2, "write error\n");
        exit(1);
      }

      // lire le feedback du père
      if (read(p2[0], &feedback, sizeof(feedback)) != sizeof(feedback)) {
        fprintf(2, "read error\n");
        exit(1);
      }

      if (feedback == 1) {
        max = guess - 1;
      } else if (feedback == -1) {
        min = guess + 1;
      } else {
        break;
      }
    }

    close(p1[1]);
    close(p2[0]);
  }
  exit(0);
}
