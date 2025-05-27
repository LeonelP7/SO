#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *funcionHilos(void *args);

pthread_cond_t condSec = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPpal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int rondas = 0, nHilos = 0, rows = 0, cols = 0;
int **matriz = NULL;
int **matrizAux = NULL;
int terminados = 0;

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Faltan args :(\n");
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(argv[1], "r");
  pthread_t *pidHilos = NULL;
  fscanf(f, "%d", &rondas);
  fscanf(f, "%d", &nHilos);
  fscanf(f, "%d", &rows);
  fscanf(f, "%d", &cols);
  pidHilos = (pthread_t *)calloc(nHilos, sizeof(pthread_t));

  matriz = calloc(rows, sizeof(int *));
  matrizAux = calloc(rows, sizeof(int *));
  if (matriz == NULL || matrizAux == NULL) {
    perror("error al inicializar las matrices :(");
  }
  printf("Matriz inicial: \n");
  for (int i = 0; i < rows; i++) {
    matriz[i] = calloc(cols, sizeof(int));
    matrizAux[i] = calloc(cols, sizeof(int));
    for (int j = 0; j < cols; j++) {
      fscanf(f, "%d", &matriz[i][j]);
      printf("[%d] ", matriz[i][j]);
    }
    printf("\n");
  }
  fclose(f);

  for (int i = 0; i < nHilos; i++) {
    int *arg = calloc(1, sizeof(int *));
    if (arg == NULL) {
      perror("error al crear args");
    }
    *arg = i;
    pthread_create(&pidHilos[i], NULL, funcionHilos, (void *)arg);
  }

  for (int rondaAct = 0; rondaAct < rondas; rondaAct++) {
    pthread_mutex_lock(&mutex);
    while (terminados < nHilos) {
      pthread_cond_wait(&condSec, &mutex);
    }

    printf("Ronda actual %d:\n", rondaAct + 1);
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        matriz[i][j] = matrizAux[i][j];
        printf("[%d] ", matriz[i][j]);
      }
      printf("\n");
    }
    terminados = 0;
    pthread_cond_broadcast(&condPpal);
    pthread_mutex_unlock(&mutex);
  }

  for (int i = 0; i < nHilos; i++) {
    pthread_join(pidHilos[i], NULL);
  }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condPpal);
  pthread_cond_destroy(&condSec);
  free(matriz);
  free(matrizAux);
  free(pidHilos);

  return EXIT_SUCCESS;
}

void *funcionHilos(void *args) {
  int indice = (*(int *)args);
  free(args);
  int nExpuestos = 0, nVerificados = 0;
  float pVerificacion = 0;
  int verificado = 0;

  for (int rondActual = 0; rondActual < rondas; rondActual++) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        //printf("hola\n");
        for (int i2 = (((i - 1) < 0) ? 0 : i - 1);
             i2 < (((i + 2) > rows) ? rows : i + 2); i2++) {
          for (int j2 = (((j - 1) < 0) ? 0 : j - 1);
               j2 < (((j + 2) > cols) ? cols : j + 2); j2++) {
            if (matriz[i2][j2] == 1 || matriz[i2][j2] == 2) {
              nExpuestos++;
            }
            if (matriz[i2][j2] == 2) {
              nVerificados++;
            }
          }
        }
        switch (indice) {
          case 0:
            if (matriz[i][j] == 0 && nExpuestos > 1) {
              matrizAux[i][j] = 1;
            }else {
              matrizAux[i][j] = matriz[i][j];
            }
            break;
          case 1:
            if (matriz[i][j] == 1) {
              pVerificacion = 0.15 + (nVerificados * 0.05);
              verificado = ((float)rand() / RAND_MAX) < pVerificacion;
              matrizAux[i][j] = verificado ? 2 : 1;
            }
            break;
        }
        nExpuestos = 0;
        nVerificados = 0;
      }
    }
    pthread_mutex_lock(&mutex);
    terminados++;
    if (terminados == nHilos) {
      pthread_cond_signal(&condSec);
    }
    pthread_cond_wait(&condPpal, &mutex);
    pthread_mutex_unlock(&mutex);
  }
}