#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_EAT 3
#define N_PHILO 5

sem_t *semas = NULL;

void *functionThreads(void *args);

int main(int argc, char const *argv[]) {

  pthread_t *pidThreads = (pthread_t *)calloc(N_PHILO, sizeof(pthread_t));
  clock_t tIni, tFin;
  double secs = 0;
  semas = (sem_t *)calloc(N_PHILO, sizeof(sem_t));
  srand(time(NULL));
  for (int i = 0; i < N_PHILO; i++) {
    int *arg = calloc(1, sizeof(int));
    *arg = i;
    pthread_create(&pidThreads[i], NULL, functionThreads, arg);
    sem_init(&semas[i], 0, 1);
  }
  tIni = clock();
  for (int i = 0; i < N_PHILO; i++) {
    pthread_join(pidThreads[i], NULL);
  }
  tFin = clock();
  secs = (double)(tFin-tIni)/CLOCKS_PER_SEC;
  printf("Tiempo de ejecucion: %.16g\n",secs*1000);
  for (int i = 0; i < N_PHILO; i++) {
    sem_destroy(&semas[i]);
  }
  free(pidThreads);
  free(semas);
  return EXIT_SUCCESS;
}

void *functionThreads(void *args) {
  int me = *((int *)args);
  free(args);
  int nEat = 0;
  int ady = (me + 1) % N_PHILO;
  int haveR = 0, haveL = 0;
  while (nEat < N_EAT) {
    int segundos = rand() % 3 + 1;
    printf("Philosopher %d is thinking\n", me + 1);
    sleep(segundos);

    sem_wait(&semas[me]);
    printf("Philosopher %d picked left fork\n", me + 1);
    sem_wait(&semas[ady]);
    printf("Philosopher %d picked right fork\n", me + 1);
    segundos = rand() % 3 + 1;
    printf("Philosopher %d is eating\n", me + 1);
    sleep(segundos);
    nEat++;
    sem_post(&semas[me]);
    sem_post(&semas[ady]);
  }
  printf("Philosopher %d is done eating :)\n",me+1);
  pthread_exit(EXIT_SUCCESS);
}