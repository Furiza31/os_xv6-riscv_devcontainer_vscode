#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/fcntl.h"
#include "../user/user.h"
#include "../user/pthread.h"

#define NUM_STOCKS 10
#define NUM_TRADERS 1
#define INITIAL_CAPITAL 10000
#define MIN_STOCK_PRICE 100
#define MAX_STOCK_PRICE 200
#define BUY 0
#define SELL 1

typedef struct {
  int stockPrices[NUM_STOCKS];
  pthread_mutex_t mutex;
} Market;

typedef struct {
  int id;
  int capital;
  Market *market;
} Trader;

Market market;
Trader traders[NUM_TRADERS];

int randInt(int min, int max) {
  uint seed = uptime();
  srand(seed);
  return (rand() % (max - min + 1)) + min;
}

void initializeMarket() {
  for (int i = 0; i < NUM_STOCKS; i++) {
    market.stockPrices[i] = randInt(MIN_STOCK_PRICE, MAX_STOCK_PRICE);
  }
  pthread_mutex_init(&market.mutex, 0);
}

void updateStockPrice(int stockId) {

  int variation = randInt(-10, 10);
  market.stockPrices[stockId] += variation;

  if (market.stockPrices[stockId] < MIN_STOCK_PRICE) {
    market.stockPrices[stockId] = MIN_STOCK_PRICE;
  } else if (market.stockPrices[stockId] > MAX_STOCK_PRICE) {
    market.stockPrices[stockId] = MAX_STOCK_PRICE;
  }
}

void *trade(void *arg) {
  Trader *trader = (Trader *)arg;

  while (trader->capital > 0) {

    int action = randInt(0, 1);

    int stockId = randInt(0, NUM_STOCKS - 1);

    pthread_mutex_lock(&trader->market->mutex);

    int stockPrice = trader->market->stockPrices[stockId];

    if (action == BUY) {

      if (trader->capital >= stockPrice) {
        trader->capital -= stockPrice;
        printf("Trader %d a achete le stock %d au prix de %d. Capital restant: "
               "%d\n",
               trader->id, stockId, stockPrice, trader->capital);
      }
    } else {

      trader->capital += stockPrice;
      printf(
          "Trader %d a vendu le stock %d au prix de %d. Nouveau capital: %d\n",
          trader->id, stockId, stockPrice, trader->capital);
    }

    updateStockPrice(stockId);

    pthread_mutex_unlock(&trader->market->mutex);

    sleep(5);
  }

  return 0;
}

void *crashMarket(void *arg) {

  sleep(100);

  printf("\n*** ALERTE: CRASH BOURSIER IMMINENT! ***\n\n");

  pthread_mutex_lock(&market.mutex);

  int numCrashing = NUM_STOCKS * 4 / 10;
  for (int i = 0; i < numCrashing; i++) {
    int stockId = randInt(0, NUM_STOCKS - 1);
    int oldPrice = market.stockPrices[stockId];
    market.stockPrices[stockId] = oldPrice / 5;
    printf("CRASH: Le stock %d s'effondre de %d à %d!\n", stockId, oldPrice,
           market.stockPrices[stockId]);
  }

  pthread_mutex_unlock(&market.mutex);

  return 0;
}

int main(int argc, char *argv[]) {
  pthread_t threads[NUM_TRADERS];
  pthread_t crashThread;

  printf("SIMULATION DE TRADING AVEC %d TRADERS\n\n", NUM_TRADERS);

  initializeMarket();

  for (int i = 0; i < NUM_TRADERS; i++) {
    traders[i].id = i;
    traders[i].capital = INITIAL_CAPITAL;
    traders[i].market = &market;

    if (pthread_create(&threads[i], 0, trade, &traders[i]) != 0) {
      printf("Erreur lors de la création du thread trader %d\n", i);
      exit(1);
    }
  }

  if (pthread_create(&crashThread, 0, crashMarket, 0) != 0) {
    printf("Erreur lors de la création du thread de crash\n");
    exit(1);
  }

  for (int i = 0; i < NUM_TRADERS; i++) {
    pthread_join(threads[i], 0);
  }

  pthread_join(crashThread, 0);

  pthread_mutex_destroy(&market.mutex);

  exit(0);
}