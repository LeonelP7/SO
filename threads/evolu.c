#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  double f1;
  double f2;
  double fitness;
} Individuo;

Individuo *vect = NULL;
int nHilos = 0, nIteraciones = 0, vectSize = 0;
int terminados = 0;
int max = 20, min = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condSec = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPpal = PTHREAD_COND_INITIALIZER;

void *funcionHilos(void *args);

int compare(const void*a, const void*b);

int main(int argc, char const *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "Faltan args :(\n|");
    exit(EXIT_FAILURE);
  }


  pthread_t *pidHilos = NULL;
  nHilos = strtol(argv[1], NULL, 10);
  nIteraciones = strtol(argv[2], NULL, 10);
  vectSize = strtol(argv[3], NULL, 10);
  pidHilos = calloc(nHilos, sizeof(pthread_t));

  vect = calloc(vectSize, sizeof(Individuo));
  srand(time(NULL));  // Inicializar la semilla

  for (int i = 0; i < vectSize; i++) {
    Individuo *individuo = calloc(1, sizeof(Individuo));
    individuo->f1 = (rand() % max) + min;
    individuo->f2 = (rand() % max) + min;
    // printf("Individuo %d:\n",i);
    // printf("f1:%.2lf\n",individuo->f1);
    // printf("f2:%.2lf\n",individuo->f2);
    individuo->fitness = (individuo->f1 * individuo->f2) / 2;
    vect[i] = *individuo;
    free(individuo);
  }

  for (int i = 0; i < nHilos; i++) {
    int *args = calloc(1, sizeof(int));
    pthread_create(&pidHilos[i], NULL, funcionHilos, args);
  }

  for (int i = 1; i < nIteraciones+1; i++) {
    pthread_mutex_lock(&mutex);
    while (terminados < nHilos) {
      pthread_cond_wait(&condSec, &mutex);
    }

    qsort(vect,vectSize,sizeof(Individuo),compare);

    printf("Los 10 mas aptos en la iteracion %d:\n",i);
    for (int vi = 0; vi < 10; vi++)
    {
        printf("Individuo %d con la aptitud %.2lf\n",vi+1,vect[vi].fitness);
    }
    terminados = 0;
    
    pthread_cond_broadcast(&condPpal);
    pthread_mutex_unlock(&mutex);
  }

  for (int i = 0; i < nHilos; i++)
  {
    pthread_join(pidHilos[i],NULL);
  }
  
  return 0;
}

void *funcionHilos(void *args) {
  int iteracion = (*(int *)args);

  float delta = ceil(vectSize / nHilos);
  float ini = delta * iteracion;
  float fin = (ini + delta) > vectSize ? vectSize : ini + delta;

  for (int it = 0; it < nIteraciones; it++) {
    for (int i = ini; i < fin; i++) {
      vect[i].f1 += (rand() % max) + min;
      vect[i].f2 += (rand() % max) + min;
      vect[i].fitness = (vect[i].f1 * vect[i].f2) / 2;
    }
    //printf("Iteracion: %d\n",it);
    pthread_mutex_lock(&mutex);
    terminados++;
    if (terminados==nHilos)
    {
        pthread_cond_signal(&condSec);
    }
    pthread_cond_wait(&condPpal, &mutex);
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(EXIT_SUCCESS);
}

int compare(const void*a, const void*b){
    Individuo ia = (*(Individuo*)a);
    Individuo ib = (*(Individuo*)b);
    return ib.fitness-ia.fitness;
}
