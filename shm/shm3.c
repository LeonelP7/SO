#include <math.h>
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
  int idA, idB, idC;
  int rowsA = 3, colsA = 3;
  int rowsB = 3, colsB = 3;

  if (colsA != rowsB) {
    printf("Esa vuelta no se puede :(\n");
    return 1;
  }

  idA = shmget(IPC_PRIVATE, sizeof_dm(rowsA, colsA, sizeof(int)),
               IPC_CREAT | 0600);
  idB = shmget(IPC_PRIVATE, sizeof_dm(rowsB, colsB, sizeof(int)),
               IPC_CREAT | 0600);
  idC = shmget(IPC_PRIVATE, sizeof_dm(rowsA, colsB, sizeof(int)),
               IPC_CREAT | 0600);

  matrizA = shmat(idA, NULL, 0);
  matrizB = shmat(idB, NULL, 0);
  matrizC = shmat(idC, NULL, 0);

  create_index((void *)matrizA, rowsA, colsA, sizeof(int));
  create_index((void *)matrizB, rowsB, colsB, sizeof(int));
  create_index((void *)matrizC, rowsA, colsB, sizeof(int));

  for (int i = 0; i < rowsA; i++) {
    for (int j = 0; j < colsA; j++) {
      matrizA[i][j] = (i * colsA) + j;
    }
  }

  printf("Matriz A:\n");
  for (int i = 0; i < rowsA; i++) {
    for (int j = 0; j < colsA; j++) {
      printf("[%d] ", matrizA[i][j]);
    }
    printf("\n");
  }

  for (int i = 0; i < rowsB; i++) {
    for (int j = 0; j < colsB; j++) {
      matrizB[i][j] = ((i * colsA) + j) * 2;
    }
  }

  printf("Matriz B:\n");
  for (int i = 0; i < rowsB; i++) {
    for (int j = 0; j < colsB; j++) {
      printf("[%d] ", matrizB[i][j]);
    }
    printf("\n");
  }

  int nHijos;
  int nProceso;
  if (colsB < rowsA) {
    nHijos = (int)ceil(((double)colsB) / 2);
  } else {
    nHijos = (int)ceil(((double)rowsA) / 2);
  }

  printf("n hijos:%d\n", nHijos);

  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  if (nProceso == nHijos) {
    shmdt(matrizA);
    shmdt(matrizB);
    for (int i = 0; i < nHijos; i++) {
      wait(NULL);
    }

    printf("Matriz C:\n");
    for (int i = 0; i < rowsA; i++) {
      for (int j = 0; j < colsB; j++) {
        printf("[%d] ", matrizC[i][j]);
      }
      printf("\n");
    }

    shmdt(matrizC);
    shmctl(idA, IPC_RMID, NULL);
    shmctl(idB, IPC_RMID, NULL);
    shmctl(idC, IPC_RMID, NULL);
  } else if (nProceso != nHijos) {
    // printf("hola\n");
    // printf("hola\n");
    printf("nProceso: %d\n",nProceso);
    for (int i = nProceso; i < (rowsA-nProceso); i++) {
      if (i == nProceso || i == ((rowsA - 1) - nProceso)) {
        for (int j = nProceso; j < (colsB-nProceso); j++) {
          // printf("hola\n");

          matrizC[i][j] = 0;
          for (int k = 0; k < colsA; k++) {
            matrizC[i][j] += matrizA[i][k] * matrizB[k][j];
          }
        }
      } else {
        for (int j = nProceso; j < (colsB-nProceso); j += ((colsB - 1) - nProceso-(nProceso>0?1:0) )) {
            printf("colsB: %d\n",colsB);
            printf("j: %d\n",j);
          matrizC[i][j] = 0;
          for (int k = 0; k < colsA; k++) {
            matrizC[i][j] += matrizA[i][k] * matrizB[k][j];
          }
          //j = 0;
          
        }
      }
    }
    shmdt(matrizA);
    shmdt(matrizB);
    shmdt(matrizC);
  }


  return 0;
}
