#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/fcntl.h"
#include "../user/user.h"
#include "../user/pthread.h"

#define BOARD_SIZE 9
#define EMPTY 9
#define PLAYER_X -1
#define PLAYER_O 1
#define WINNING_SUM 3

typedef struct {
  int board[BOARD_SIZE];
  int currentPlayer;
  int gameOver;
  int winner;
  
  int moveReady;
  int move;
  
  pthread_mutex_t mutex;
} GameState;

GameState gameState;

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

int abs(int x) {
  return (x < 0) ? -x : x;
}

int checkWin(int* board) {
  for (int i = 0; i < 3; i++) {
    int rowSum = board[i*3] + board[i*3 + 1] + board[i*3 + 2];
    if (abs(rowSum) == WINNING_SUM) {
      return rowSum / WINNING_SUM;
    }
  }
  
  for (int i = 0; i < 3; i++) {
    int colSum = board[i] + board[i + 3] + board[i + 6];
    if (abs(colSum) == WINNING_SUM) {
      return colSum / WINNING_SUM;
    }
  }
  
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
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board[i] == EMPTY) {
      return 0;
    }
  }
  
  return 1;
}

int isGameOver(int* board) {
  return (checkWin(board) != 0) || isDraw(board);
}

int getValidRandomMove(int* board) {
  int emptyCells[BOARD_SIZE];
  int count = 0;
  
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board[i] == EMPTY) {
      emptyCells[count++] = i;
    }
  }

  if (count == 0) {
    return -1;
  }

  uint seed = uptime();
  srand(seed);
  int r = rand();
  
  int randomIndex = r % count;
  return emptyCells[randomIndex];
}

void* playAsReferee(void* arg) {
  pthread_mutex_lock(&gameState.mutex);
  
  printf("Début de la partie!\n");
  printBoard(gameState.board);
  
  while (!isGameOver(gameState.board)) {
    gameState.moveReady = 0;
    pthread_mutex_unlock(&gameState.mutex);
    
    while (1) {
      pthread_mutex_lock(&gameState.mutex);
      if (gameState.moveReady) {
        break;
      }
      pthread_mutex_unlock(&gameState.mutex);
      sleep(5);
    }
    
    int player = gameState.currentPlayer;
    int move = gameState.move;
    
    if (move >= 0 && move < BOARD_SIZE && gameState.board[move] == EMPTY) {
      gameState.board[move] = player;
      if (player == PLAYER_X) {
        printf("Joueur A : a joué! Position %d\n", move + 1);
      } else {
        printf("Joueur B : a joué! Position %d\n", move + 1);
      }
    }
    
    printBoard(gameState.board);
    
    gameState.currentPlayer = -gameState.currentPlayer;
    
    if (isGameOver(gameState.board)) {
      gameState.gameOver = 1;
      gameState.winner = checkWin(gameState.board);
    }
    
    sleep(5);
  }
  
  if (gameState.winner == PLAYER_X) {
    printf("Bravo Joueur A: vous avez gagné la partie!\n");
  } else if (gameState.winner == PLAYER_O) {
    printf("Bravo Joueur B: vous avez gagné la partie!\n");
  } else {
    printf("C'est un match nul!\n");
  }
  
  pthread_mutex_unlock(&gameState.mutex);
  return 0;
}

void* playAsPlayerA(void* arg) {
  while (1) {
    pthread_mutex_lock(&gameState.mutex);

    if (gameState.gameOver) {
      pthread_mutex_unlock(&gameState.mutex);
      break;
    }
    
    if (gameState.currentPlayer == PLAYER_X && !gameState.moveReady) {
      gameState.move = getValidRandomMove(gameState.board);
      gameState.moveReady = 1;
    }
    
    pthread_mutex_unlock(&gameState.mutex);
    sleep(5);
  }
  
  return 0;
}

void* playAsPlayerB(void* arg) {
  while (1) {
    pthread_mutex_lock(&gameState.mutex);
    
    if (gameState.gameOver) {
      pthread_mutex_unlock(&gameState.mutex);
      break;
    }
    
    if (gameState.currentPlayer == PLAYER_O && !gameState.moveReady) {
      gameState.move = getValidRandomMove(gameState.board);
      gameState.moveReady = 1;
    }
    
    pthread_mutex_unlock(&gameState.mutex);
    sleep(5);
  }
  
  return 0;
}

int main(int argc, char *argv[])
{
  pthread_t refereeThread, playerAThread, playerBThread;
  
  for (int i = 0; i < BOARD_SIZE; i++) {
    gameState.board[i] = EMPTY;
  }
  gameState.currentPlayer = PLAYER_X;
  gameState.gameOver = 0;
  gameState.winner = 0;
  gameState.moveReady = 0;
  
  pthread_mutex_init(&gameState.mutex, 0);
  
  if (pthread_create(&refereeThread, 0, playAsReferee, 0) != 0) {
    printf("Erreur lors de la création du thread d'arbitre\n");
    exit(1);
  }
  
  if (pthread_create(&playerAThread, 0, playAsPlayerA, 0) != 0) {
    printf("Erreur lors de la création du thread du joueur A\n");
    exit(1);
  }
  
  if (pthread_create(&playerBThread, 0, playAsPlayerB, 0) != 0) {
    printf("Erreur lors de la création du thread du joueur B\n");
    exit(1);
  }
  
  pthread_join(refereeThread, 0);
  pthread_join(playerAThread, 0);
  pthread_join(playerBThread, 0);
  
  pthread_mutex_destroy(&gameState.mutex);
  
  exit(0);
}