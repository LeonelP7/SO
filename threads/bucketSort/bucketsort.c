#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int *vect = NULL;
int vectSize = 0, min_val = 1, max_val = 0, intervalo, nHilos = 0, terminados = 0;
void *funcionHilos(void *args);

typedef struct {
  int *vectR;
  int vectRSize;
} ResultadoH;


int main(int argc, char const *argv[]) {
  /*
  1er argumento: tamaño del vector principal
  2do argumento: limite superior
  3er argumento: intervalo
  */
  if (argc < 4) {
    fprintf(stderr, "Faltan args :(\n");
    exit(EXIT_FAILURE);
  }

  pthread_t *pidHilos = NULL;
  vectSize = (int)strtol(argv[1], NULL, 10);
  max_val = (int)strtol(argv[2], NULL, 10);
  intervalo = (int)strtol(argv[3], NULL, 10);
  // nHilos = (max_val - min_val) / intervalo;
  nHilos = (max_val - min_val + 1 + intervalo - 1) / intervalo;;
  if (nHilos < 1) {
    nHilos = 1;
  }
  printf("nHilos: %d\n", nHilos);

  vect = calloc(vectSize, sizeof(int));
  pidHilos = calloc(nHilos, sizeof(pthread_t));
  srand(time(NULL));

  printf("Vector inicial:\n");
  for (int i = 0; i < vectSize; i++) {
    vect[i] = (rand() % (max_val - min_val + 1)) + min_val;
    printf("[%d] ", vect[i]);
  }
  printf("\n");

  for (int i = 0; i < nHilos; i++) {
    int *args = calloc(1, sizeof(int));
    *args = i;
    if (pthread_create(&pidHilos[i], NULL, funcionHilos, args) != 0) {
      perror("error al crear hilos");
      exit(EXIT_FAILURE);
    }
  }

  ResultadoH *resultado = NULL;
  int k = 0;
  for (int i = 0; i < nHilos; i++) {
    pthread_join(pidHilos[i], (void **)&resultado);
    for (int j = 0; j < resultado->vectRSize; j++) {
      vect[k++] = resultado->vectR[j];
    }
    free(resultado->vectR);
    free(resultado);
  }
  // printf("k en el principal %d\n",k);

  printf("Resultado: \n");
  for (int i = 0; i < vectSize; i++) {
    printf("[%2d] ", vect[i]);
  }
  printf("\n");

  free(vect);
  free(pidHilos);

  return EXIT_SUCCESS;
}

void *funcionHilos(void *args) {
  int inidice = (*(int *)args);
  free(args);
  ResultadoH *resultado = calloc(1, sizeof(ResultadoH));
  resultado->vectRSize = 0;
  // int hMin = (inidice * intervalo) + ((inidice + 1) * min);
  // int hMax = ((inidice + 1) * intervalo) + ((inidice + 1) * min);
  int hMin = min_val + (inidice * intervalo);
  int hMax = hMin + intervalo - 1;
  if (hMax > max_val) hMax = max_val;

  printf("min: %d y max: %d\n", hMin, hMax);

  for (int i = 0; i < vectSize; i++) {
    if (vect[i] >= hMin && vect[i] <= hMax) {
      resultado->vectRSize++;
    }
  }
  printf("vectRSize: %d\n", resultado->vectRSize);
  resultado->vectR = calloc(resultado->vectRSize, sizeof(int));
  int k = 0;
  for (int i = 0; i < vectSize; i++) {
    if (vect[i] >= hMin && vect[i] <= hMax) {
      resultado->vectR[k] = vect[i];
      k++;
    }
  }

  printf("el hilo %d va a organizar: \n", inidice);
  for (int i = 0; i < resultado->vectRSize; i++) {
    printf("[%d] ", resultado->vectR[i]);
  }
  printf("\n");

  for (int i = 0; i < resultado->vectRSize - 1; i++) {
    for (int j = 0; j < resultado->vectRSize - i - 1; j++) {
      if (resultado->vectR[j] > resultado->vectR[j + 1]) {
        int aux = resultado->vectR[j];
        resultado->vectR[j] = resultado->vectR[j + 1];
        resultado->vectR[j + 1] = aux;
      }
    }
  }

  printf("el hilo %d organizo: \n", inidice);
  for (int i = 0; i < resultado->vectRSize; i++) {
    printf("[%d] ", resultado->vectR[i]);
  }
  printf("\n");

  pthread_exit((void *)resultado);
}