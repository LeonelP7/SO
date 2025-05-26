#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int terminaron = 0;
pthread_cond_t condSec = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPpal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

float *vect = NULL;
float *vectAux = NULL;
int nPasos;
int nHilos;
void *funcionHilos(void *args);

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Faltan args :(\n");
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(argv[1], "r");
  nHilos = 0;
  nPasos = 0;
  pthread_t *pidhilos = NULL;
  fscanf(f, "%d", &nHilos);
  fscanf(f, "%d", &nPasos);
  pidhilos = (pthread_t *)calloc(nHilos, sizeof(pthread_t));
  vect = (float *)calloc(nHilos, sizeof(float));
  vectAux = (float *)calloc(nHilos, sizeof(float));

  printf("Vector inicial: \n");
  for (int i = 0; i < nHilos; i++) {
    fscanf(f, "%f", &vect[i]);
    printf("[%4.2f] ", vect[i]);

    int *arg = malloc(sizeof(int));
    if (!arg) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    *arg = i;
    pthread_create(&pidhilos[i], NULL, funcionHilos, (void *)arg);
  }
  printf("\n");
  fclose(f);

  for (int pasoActual = 0; pasoActual < nPasos; pasoActual++) {
    pthread_mutex_lock(&mutex);

    while (terminaron < nHilos) {
      pthread_cond_wait(&condSec, &mutex);
    }

    printf("Resultado en el paso %d:\n", pasoActual + 1);
    for (int vi = 0; vi < nHilos; vi++) {
      vect[vi] = vectAux[vi];
      printf("[%4.2f] ", vect[vi]);
    }

    printf("\n");
    terminaron = 0;
    pthread_cond_broadcast(&condPpal);
    pthread_mutex_unlock(&mutex);
  }

  for (int i = 0; i < nHilos; i++) pthread_join(pidhilos[i], NULL);

  // Libera la memoria de los identificadores de los hilos
  free(pidhilos);

  // Destruye el mutex y la variable de condición
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condSec);
  pthread_cond_destroy(&condPpal);
  return 0;
}

void *funcionHilos(void *args) {
  int indice = *((int *)args);
  free(args);
  float act = vect[indice];
  float ant = ((indice)-1 < 0) ? 0 : vect[(indice)-1];
  float post = ((indice) + 1 >= nHilos) ? 0 : vect[(indice) + 1];

  for (int paso = 0; paso < nPasos; paso++) {
    act = vect[indice];
    ant = (indice - 1 < 0) ? 0 : vect[indice - 1];
    post = (indice + 1 >= nHilos) ? 0 : vect[indice + 1];
    vectAux[indice] = (ant + 2 * act + post) / 4;

    pthread_mutex_lock(&mutex);
    terminaron++;
    if (terminaron == nHilos) {
      pthread_cond_signal(&condSec);  // Despierta al padre
    }
    pthread_cond_wait(&condPpal, &mutex);
    pthread_mutex_unlock(&mutex);
  }

  pthread_exit(0);
}