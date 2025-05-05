#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define BOARD_SIZE 9
#define EMPTY 9
#define PLAYER_X -1
#define PLAYER_O 1
#define WINNING_SUM 3

// Indice des pipes
#define READ 0
#define WRITE 1

void printBoard(int* board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (i > 0 && i % 3 == 0) {
      printf("\n");
    }
    
    if (board[i] == EMPTY) {
      printf("_ ");
    } else if (board[i] == PLAYER_X) {
      printf("X ");
    } else if (board[i] == PLAYER_O) {
      printf("O ");
    }
    
    if ((i + 1) % 3 == 0) {
      printf("\n");
    }
  }
  printf("\n");
}

int checkWin(int* board) {
  // Regarde les lignes
  for (int i = 0; i < 3; i++) {
    int rowSum = board[i*3] + board[i*3 + 1] + board[i*3 + 2];
    if (abs(rowSum) == WINNING_SUM) {
      return rowSum / WINNING_SUM;
    }
  }
  
  // Regarde les colonnes
  for (int i = 0; i < 3; i++) {
    int colSum = board[i] + board[i + 3] + board[i + 6];
    if (abs(colSum) == WINNING_SUM) {
      return colSum / WINNING_SUM;
    }
  }
  
  // Regarde les diagonales
  int diag1 = board[0] + board[4] + board[8];
  int diag2 = board[2] + board[4] + board[6];
  
  if (abs(diag1) == WINNING_SUM) {
    return diag1 / WINNING_SUM;
  }
  
  if (abs(diag2) == WINNING_SUM) {
    return diag2 / WINNING_SUM;
  }
  
  return 0;
}

int isDraw(int* board) {
  int sum = 0;
  for (int i = 0; i < BOARD_SIZE; i++) {
    sum += board[i];
  }
  
  return abs(sum) == 1;
}

int isGameOver(int* board) {
  return (checkWin(board) != 0) || isDraw(board);
}

int getValidRandomMove(int* board) {
  int emptyCells[BOARD_SIZE];
  int count = 0;
  
  // Trouver les cases vides
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board[i] == EMPTY) {
      emptyCells[count++] = i;
    }
  }

  // Si aucune case vide, retourner -1
  if (count == 0) {
    return -1;
  }

  // Générer un nombre aléatoire
  uint32 r, seed = uptime();
  srand(seed);
  r = rand();
  
  // Choisir une case vide aléatoire
  int randomIndex = r % count;
  return emptyCells[randomIndex];
}

void playAsReferee(int* board, int pipeToA[2], int pipeFromA[2], 
                   int pipeToB[2], int pipeFromB[2]) {
  int currentPlayer = PLAYER_X; // X commence
  int move;
  int winner = 0;

  printBoard(board);

  while (!isGameOver(board)) {
    if (currentPlayer == PLAYER_X) {
      // Demander le coup du joueur X
      write(pipeToA[WRITE], board, sizeof(int) * BOARD_SIZE);
      read(pipeFromA[READ], &move, sizeof(int));
      
      if (move >= 0 && move < BOARD_SIZE && board[move] == EMPTY) {
        board[move] = PLAYER_X;
        printf("Joueur A : à vous de joueur! Position %d\n", move + 1);
      }
    } else {
      // Demander le coup du joueur O
      write(pipeToB[WRITE], board, sizeof(int) * BOARD_SIZE);
      read(pipeFromB[READ], &move, sizeof(int));
      
      if (move >= 0 && move < BOARD_SIZE && board[move] == EMPTY) {
        board[move] = PLAYER_O;
        printf("Joueur B : à vous de joueur! Position %d\n", move + 1);
      }
    }

    printBoard(board);
    
    currentPlayer = -currentPlayer;

    sleep(10);
  }

  winner = checkWin(board);
  if (winner == PLAYER_X) {
    printf("Bravo Joueur A: vous avez gagné la partie!\n");
  } else if (winner == PLAYER_O) {
    printf("Bravo Joueur B: vous avez gagné la partie!\n");
  } else {
    printf("C'est un match nul!\n");
  }

  // Envoyer un signal de fin aux joueurs
  int endSignal = -99;
  write(pipeToA[WRITE], &endSignal, sizeof(int));
  write(pipeToB[WRITE], &endSignal, sizeof(int));
}

void playAsPlayerA(int pipeToParent[2], int pipeFromParent[2]) {
  int board[BOARD_SIZE];
  int move;

  while (1) {
    if (read(pipeFromParent[READ], board, sizeof(int) * BOARD_SIZE) <= 0) {
      break;
    }

    if (board[0] == -99) {
      break;
    }

    move = getValidRandomMove(board);
    write(pipeToParent[WRITE], &move, sizeof(int));
  }
}

void playAsPlayerB(int pipeToParent[2], int pipeFromParent[2]) {
  int board[BOARD_SIZE];
  int move;

  while (1) {
    if (read(pipeFromParent[READ], board, sizeof(int) * BOARD_SIZE) <= 0) {
      break;
    }

    if (board[0] == -99) {
      break;
    }

    move = getValidRandomMove(board);
    write(pipeToParent[WRITE], &move, sizeof(int));
  }
}


int main(int argc, char *argv[])
{
  int board[BOARD_SIZE];
  int pipeParentToA[2], pipeAToParent[2];
  int pipeParentToB[2], pipeBToParent[2];
  int pid1, pid2;

  // Créer le tableau de jeu
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[i] = EMPTY;
  }

  // Créer les pipes
  if (pipe(pipeParentToA) < 0 || pipe(pipeAToParent) < 0 ||
      pipe(pipeParentToB) < 0 || pipe(pipeBToParent) < 0) {
    printf("Erreur lors de la création des pipes\n");
    exit(1);
  }

  // Joueur (A - X)
  pid1 = fork();
  if (pid1 < 0) {
    printf("Erreur lors de la création du joueur X\n");
    exit(1);
  }

  if (pid1 == 0) {
    // Child process (Joueur A - X)
    close(pipeParentToA[WRITE]);
    close(pipeAToParent[READ]);
    close(pipeParentToB[READ]);
    close(pipeParentToB[WRITE]);
    close(pipeBToParent[READ]);
    close(pipeBToParent[WRITE]);

    playAsPlayerA(pipeAToParent, pipeParentToA);
    exit(0);
  }

  // Joueur (B - O)
  pid2 = fork();
  if (pid2 < 0) {
    printf("Erreur lors de la création du joueur B\n");
    kill(pid1);
    exit(1);
  }

  if (pid2 == 0) {
    // Child process (Joueur B - O)
    close(pipeParentToB[WRITE]);
    close(pipeBToParent[READ]);
    close(pipeParentToA[READ]);
    close(pipeParentToA[WRITE]);
    close(pipeAToParent[READ]);
    close(pipeAToParent[WRITE]);

    playAsPlayerB(pipeBToParent, pipeParentToB);
    exit(0);
  }

  // Parent process (Joueur - Referee)
  close(pipeParentToA[READ]);
  close(pipeAToParent[WRITE]);
  close(pipeParentToB[READ]);
  close(pipeBToParent[WRITE]);

  playAsReferee(board, pipeParentToA, pipeAToParent, pipeParentToB, pipeBToParent);

  // Attendre la fin des joueurs
  wait(0);
  wait(0);

  exit(0);
}