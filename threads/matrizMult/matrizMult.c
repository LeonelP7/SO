#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// funcion para inicializar los espacios de una matriz
void getMatrixSpace(void ***m, int rows, int cols, size_t sizeElement);
void fillMatrixFromFile(int **m, int rows, int cols, FILE *f);
void *functionSecThreads(void *arg);

struct DataT {
  int rows;
  int cols;
  int i;
};

int **m;
int **matrizA;
int **matrizB;
int main(int argc, char const *argv[]) {

  if (argc < 2) {
    fprintf(stdout, "Faltan argumentos :(\n");
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(argv[1], "r");
  int rows = 0, cols = 0;
  fscanf(f, "%d", &rows);
  fscanf(f, "%d", &cols);
  getMatrixSpace((void ***)&m, rows, cols, sizeof(int));
  getMatrixSpace((void ***)&matrizA, rows, cols, sizeof(int));
  getMatrixSpace((void ***)&matrizB, rows, cols, sizeof(int));
  //fillMatrixFromFile(m, rows, cols, f);
  fillMatrixFromFile(matrizA, rows, cols, f);
  fillMatrixFromFile(matrizB, rows, cols, f);
  fclose(f);

  int nHilos = 3;
  pthread_t tid[nHilos];
  struct DataT *dataT;
  // Create the threads
  for (int i = 0; i < nHilos; i++) {
    dataT = (struct DataT *)malloc(sizeof(struct DataT));
    if (dataT == NULL) {
      perror("Error en malloc dataT");
      exit(EXIT_FAILURE);
    }
    dataT->rows = rows;
    dataT->cols = cols;
    dataT->i = i;
    pthread_create(&tid[i], NULL, functionSecThreads, (void *)dataT);
  }

  // Espera a que todos los hilos secundarios terminen
  for (int i = 0; i < nHilos; i++) {
    pthread_join(tid[i], NULL);
  }

  printf("Resultado:\n");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("[%4d]", m[i][j]);
    }
    printf("\n");
  }


  return EXIT_SUCCESS;
}

void getMatrixSpace(void ***m, int rows, int cols, size_t sizeElement) {
  *m = malloc(rows * sizeof(void *));
  if (*m == NULL) {
    fprintf(stderr, "Error al inicializar la matriz :(");
    exit(1);
  }
  for (int i = 0; i < rows; i++) {
    (*m)[i] = calloc(cols, sizeof(sizeElement));
    if ((*m)[i] == NULL) {
      fprintf(stderr, "Error al inicializar la fila :(");
      exit(1);
    }
  }
}

void fillMatrixFromFile(int **m, int rows, int cols, FILE *f) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fscanf(f, "%d", &m[i][j]);
    }
  }
}

void *functionSecThreads(void *arg) {
  struct DataT *dataT = (struct DataT *)arg;

  for (int i = 0; i < dataT->rows; i++) {
    for (int j = 0; j < dataT->cols; j++) {

      switch (dataT->i) {
        case 0:
          if (i > j) {
            m[i][j] = 0;
            for (int k = 0; k < dataT->cols; k++) {
              m[i][j] += matrizA[i][k] * matrizB[k][j];
            }
          }
          break;

        case 1:
          if (i == j) {
            m[i][j] = 0;
            for (int k = 0; k < dataT->cols; k++) {
              m[i][j] += matrizA[i][k] * matrizB[k][j];
            }
          }
          break;
        case 2:
          if (i < j) {
            m[i][j] = 0;
            for (int k = 0; k < dataT->cols; k++) {
              m[i][j] += matrizA[i][k] * matrizB[k][j];
            }
          }
          break;
      }
    }
  }
  free(dataT);
  pthread_exit(0);
}
