#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void create_index(void **m, int rows, int cols, size_t sizeElement) {
  int i;
  size_t sizeRow = cols * sizeElement;
  m[0] = m + rows;
  for (i = 1; i < rows; i++) {
    m[i] = (m[i - 1] + sizeRow);
  }
}

unsigned int sizeof_dm(int rows, int cols, size_t sizeElement) {
  size_t size;
  size = rows * sizeof(void *);         // indexSize
  size += (cols * rows * sizeElement);  // Data size
  return size;
}

int main(int argc, char const *argv[]) {
  int **matrizA, **matrizB, **matrizC;
  int idMatrizA, idMatrizB, idMatrizC;
  int rows = 3, cols = 3, nProceso;

  size_t sizeMatrix = sizeof_dm(rows, cols, sizeof(int));
  idMatrizA = shmget(IPC_PRIVATE, sizeMatrix, IPC_CREAT | S_IRUSR | S_IWUSR);
  idMatrizB = shmget(IPC_PRIVATE, sizeMatrix, IPC_CREAT | S_IRUSR | S_IWUSR);
  idMatrizC = shmget(IPC_PRIVATE, sizeMatrix, IPC_CREAT | S_IRUSR | S_IWUSR);

  matrizA = shmat(idMatrizA, NULL, 0);
  matrizB = shmat(idMatrizB, NULL, 0);
  matrizC = shmat(idMatrizC, NULL, 0);

  create_index((void *)matrizA, rows, cols, sizeof(double));
  create_index((void *)matrizB, rows, cols, sizeof(double));
  create_index((void *)matrizC, rows, cols, sizeof(double));


  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {

      matrizA[i][j] = (i * cols) + j;
      matrizB[i][j] = ((i * cols) + j);
    }
  }

  printf("Matriz A:\n");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {

      printf("[%d]", matrizA[i][j]);
    }
    printf("\n");
  }


  for (nProceso = 0; nProceso < 3; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  if (nProceso == 3) {
    shmdt(matrizA);
    shmdt(matrizB);

    for (int i = 0; i < nProceso; i++) {
      wait(NULL);
    }

    printf("Matriz C:\n");
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {

        printf("[%d]", matrizC[i][j]);
      }
      printf("\n");
    }

    shmdt(matrizC);
    shmctl(idMatrizA, IPC_RMID, NULL);
    shmctl(idMatrizB, IPC_RMID, NULL);
    shmctl(idMatrizC, IPC_RMID, NULL);
  } else if (nProceso == 0) {

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (i < j) {
          matrizC[i][j] = 0;
          for (int k = 0; k < cols; k++) {
            matrizC[i][j] += matrizA[i][k] * matrizB[k][j];
          }
        }
      }
    }
    shmdt(matrizA);
    shmdt(matrizB);
    shmdt(matrizC);

  } else if (nProceso == 1) {

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (i == j) {
          matrizC[i][j] = 0;
          for (int k = 0; k < cols; k++) {
            matrizC[i][j] += matrizA[i][k] * matrizB[k][j];
          }
        }
      }
    }
    shmdt(matrizA);
    shmdt(matrizB);
    shmdt(matrizC);

  } else {

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (i > j) {
          matrizC[i][j] = 0;
          for (int k = 0; k < cols; k++) {
            matrizC[i][j] += matrizA[i][k] * matrizB[k][j];
          }
        }
      }
    }
    shmdt(matrizA);
    shmdt(matrizB);
    shmdt(matrizC);
  }
  return EXIT_SUCCESS;
}
