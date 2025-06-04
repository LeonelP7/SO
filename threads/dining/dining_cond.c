#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_EAT 3
#define N_PHILO 5

pthread_cond_t *cond = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int *forks = NULL;

void *functionThreads(void *args);

int main(int argc, char const *argv[]) {

  pthread_t *pidThreads = (pthread_t *)calloc(N_PHILO, sizeof(pthread_t));
  clock_t tIni, tFin;
  double secs = 0;
  forks = (int *)calloc(N_PHILO, sizeof(int));
  cond = (pthread_cond_t *)calloc(N_PHILO, sizeof(pthread_cond_t));
  srand(time(NULL));
  tIni = clock();
  for (int i = 0; i < N_PHILO; i++) {
    forks[i] = 0;
    int *arg = calloc(1, sizeof(int));
    *arg = i;
    pthread_create(&pidThreads[i], NULL, functionThreads, arg);
    pthread_cond_init(&cond[i], NULL);
  }
  for (int i = 0; i < N_PHILO; i++) {
    pthread_join(pidThreads[i], NULL);
  }
  tFin = clock();
  secs = (double)(tFin-tIni)/CLOCKS_PER_SEC;
  printf("Tiempo de ejecucion: %.16g\n",secs*1000);
  for (int i = 0; i < N_PHILO; i++) {
    pthread_cond_destroy(&cond[i]);
  }
  pthread_mutex_destroy(&mutex);
  free(pidThreads);
  free(forks);
  free(cond);
  return EXIT_SUCCESS;
}

void *functionThreads(void *args) {
  int me = *((int *)args);
  free(args);
  int nEat = 0;
  int left = ((me - 1) < 0) ? N_PHILO - 1 : me - 1;
  int right = (me + 1) % N_PHILO;
  int haveR = 0, haveL = 0;
  while (nEat < N_EAT) {
    int segundos = rand() % 3 + 1;
    printf("Philosopher %d is thinking\n", me + 1);
    sleep(segundos);

    while (!(haveR && haveL)) {
      pthread_mutex_lock(&mutex);
      if (!haveL && !forks[me]) {
        haveL = 1;
        forks[me] = 1;
        printf("Philosopher %d picked left fork\n", me + 1);
      }
      if (!haveR && !forks[right]) {
        haveR = 1;
        forks[right] = 1;
        printf("Philosopher %d picked right fork\n", me + 1);
      }
      if (!(haveR && haveL)) {
        pthread_cond_wait(&cond[me], &mutex);
      }
      pthread_mutex_unlock(&mutex);
    }

    segundos = rand() % 3 + 1;
    printf("Philosopher %d is eating\n", me + 1);
    sleep(segundos);
    nEat++;
    pthread_mutex_lock(&mutex);
    forks[me] = 0;
    forks[right] = 0;
    haveL = 0;
    haveR = 0;
    pthread_cond_signal(&cond[left]);
    pthread_cond_signal(&cond[right]);
    pthread_mutex_unlock(&mutex);
  }
  printf("Philosopher %d is done eating :)\n", me + 1);
  pthread_exit(EXIT_SUCCESS);
}