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

  if (argc < 2) {
    printf("Faltan argumentos :(\n");
    exit(1);
  }

  FILE *f = fopen(argv[1], "r");

  int tam = 0;
  fscanf(f, "%d", &tam);

  int nPasos = 0;
  fscanf(f, "%d", &nPasos);

  int idShm = shmget(IPC_PRIVATE, sizeof_dm(nPasos + 2, tam, sizeof(float)),
                     IPC_CREAT | 0600);
  float **matriz = shmat(idShm, NULL, 0);
  create_index((void *)matriz, nPasos + 2, tam, sizeof(float));

  printf("matriz[0]:\n");
  for (int i = 0; i < tam; i++) {
    fscanf(f, "%f", &matriz[0][i]);
    matriz[nPasos+1][i] = 1;
    printf("vect[%4.1f] ", matriz[0][i]);
  }
  printf("\n");

  int nProceso;
  for (nProceso = 0; nProceso < tam; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  if (nProceso == tam) {
    for (int i = 0; i < tam; i++) {
      wait(NULL);
    }

    printf("Resultado:\n");
    for (int i = 1; i < nPasos + 1; i++) {
      for (int j = 0; j < tam; j++) {
        printf("[%4.1f]", matriz[i][j]);
      }
      printf("\n");
    }

    shmdt(matriz);
    shmctl(idShm, IPC_RMID, NULL);
  } else {

    float anterior;
    float posterior;

    for (int i = 1; i < nPasos+2; i++) {
      if ((nProceso - 1) < 0) {
        while (matriz[nPasos+1][nProceso + 1] < i) {
            //printf("Esperando al posterior\n");
        }
      } else if ((nProceso + 1) >= tam) {
        while (matriz[nPasos+1][nProceso - 1] < i) {
            //printf("Esperando al anterior\n");
        }
      } else {
        while (matriz[nPasos+1][nProceso + 1] < i ||
               matriz[nPasos+1][nProceso - 1] < i) {

                //printf("Esperando a ambos\n");
        }
      }
      anterior = (nProceso - 1) < 0 ? 0 : matriz[i - 1][nProceso - 1];
      posterior = (nProceso + 1) >= tam ? 0 : matriz[i - 1][nProceso + 1];
      matriz[i][nProceso] = ((anterior) + 2 * matriz[i-1][nProceso] + posterior) / 4;
      matriz[nPasos+1][nProceso] = i+1;
    }
    shmdt(matriz);
  }
  return 0;
}
