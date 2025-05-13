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
  if (argc < 3) {
    printf("Faltan argumentos :(\n");
    exit(1);
  }

  int nHijos = (int)strtol(argv[1], NULL, 10);
  FILE *f = fopen(argv[2], "r");
  int rows = 0;
  int cols = 0;
  fscanf(f, "%d", &rows);
  fscanf(f, "%d", &cols);

  int idMatriz1 =
      shmget(IPC_PRIVATE, sizeof_dm(rows, cols, sizeof(int)), IPC_CREAT | 0600);
  int idMatriz2 = shmget(IPC_PRIVATE, sizeof_dm(rows, cols, sizeof(float)),
                         IPC_CREAT | 0600);
  int **m1 = shmat(idMatriz1, NULL, 0);
  float **m2 = shmat(idMatriz2, NULL, 0);

  create_index((void *)m1, rows, cols, sizeof(int));
  create_index((void *)m2, rows, cols, sizeof(float));

  printf("Matriz:\n");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fscanf(f, "%d", &m1[i][j]);
      printf("[%4d]", m1[i][j]);
    }
    printf("\n");
  }
  fclose(f);

  int nProceso;
  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  if (nProceso == nHijos) {
    shmdt(m1);

    for (int i = 0; i < nHijos; i++) {
      wait(NULL);
    }

    printf("Resultado:\n");
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        printf("[%5.1f]", m2[i][j]);
      }
      printf("\n");
    }
    shmdt(m2);
    shmctl(idMatriz1, IPC_RMID, NULL);
    shmctl(idMatriz2, IPC_RMID, NULL);
  } else {

    int delta = (int)ceil((float)rows / (nHijos));
    int ini = delta * nProceso;
    if (ini >= rows) {
      exit(1);
    }
    int fin = ((ini + delta) > rows) ? rows : ini + delta;

    int suma;

    for (int i = ini; i < fin; i++) {
      for (int j = 0; j < cols; j++) {
        suma = 0;
        for (int i2 = ((i - 1) < 0 ? 0 : i - 1); i2 < ((i + 2) >= rows ? rows : i + 2); i2++) {
          for (int j2 = ((j - 1) < 0 ? 0 : j - 1);j2 < ((j + 2) >= cols ? cols : j + 2); j2++) {
            suma += m1[i2][j2];
          }
        }
        m2[i][j] = (float)suma / 9;
      }
    }
    shmdt(m1);
    shmdt(m2);
  }
  return 0;
}
