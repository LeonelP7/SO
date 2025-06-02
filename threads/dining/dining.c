#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_EAT 3
#define N_PHILO 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int *forks = NULL;

void *functionThreads(void *args);

int main(int argc, char const *argv[]) {

  pthread_t *pidThreads = (pthread_t *)calloc(N_PHILO, sizeof(pthread_t));
  forks = (int *)calloc(N_PHILO, sizeof(int));
  srand(time(NULL));
  for (int i = 0; i < N_PHILO; i++) {
    forks[i] = 0;
    int *arg = calloc(1, sizeof(int));
    *arg = i;
    pthread_create(&pidThreads[i], NULL, functionThreads, arg);
  }

  for (int i = 0; i < N_PHILO; i++) {
    pthread_join(pidThreads[i], NULL);
  }

  free(pidThreads);
  free(forks);
  return EXIT_SUCCESS;
}

void *functionThreads(void *args) {
  int me = *((int *)args);
  free(args);
  int nEat = 0;
  //int ady = (me+1>=N_PHILO)?0:me+1;
  int ady = (me + 1) % N_PHILO;
  int haveR = 0, haveL = 0;
  while (nEat < N_EAT) {
    int segundos = rand() % 3 + 1;
    printf("Philosopher %d is thinking\n",me+1);
    sleep(segundos);

    while (!(haveR && haveL))
    {
        pthread_mutex_lock(&mutex);
        if (!haveL && !forks[me])
        {
            haveL = 1;
            forks[me] = 1;
            printf("Philosopher %d picked left fork\n",me+1);
        }

        if (!haveR && !forks[ady])
        {
            haveR = 1;
            forks[ady] = 1;
            printf("Philosopher %d picked right fork\n",me+1);
        }
        pthread_mutex_unlock(&mutex);
        if (!(haveR && haveL))
        {
            usleep(1000);
        }
    }
    
    segundos = rand() % 3 + 1;
    printf("Philosopher %d is eating\n",me+1);
    sleep(segundos);
    nEat++;
    pthread_mutex_lock(&mutex);
    forks[me] = 0;
    forks[ady] = 0;
    haveL = 0;
    haveR = 0;
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(EXIT_SUCCESS);
}
