#include <math.h>
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

void manejador(int sig) {}

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

  int rondas, nHijos, filas, cols;
  fscanf(f, "%d", &rondas);
  fscanf(f, "%d", &nHijos);
  fscanf(f, "%d", &filas);
  fscanf(f, "%d", &cols);

  int idShm = shmget(IPC_PRIVATE, sizeof_dm(filas, cols, sizeof(int)),
                     IPC_CREAT | 0600);
  int **matriz = shmat(idShm, NULL, 0);
  int idShm2 = shmget(IPC_PRIVATE, sizeof_dm(filas, cols, sizeof(int)),
                      IPC_CREAT | 0600);
  int **matrizAux = shmat(idShm2, NULL, 0);

  signal(SIGUSR1, manejador);

  create_index((void *)matriz, filas, cols, sizeof(int));
  create_index((void *)matrizAux, filas, cols, sizeof(int));

  printf("Matriz:\n");
  for (int i = 0; i < filas; i++) {
    for (int j = 0; j < cols; j++) {
      fscanf(f, "%d", &matriz[i][j]);
      printf("[%4d]", matriz[i][j]);
    }
    printf("\n");
  }

  int nProceso;
  pid_t idHjos[nHijos];
  int tubs[nHijos][2];
  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    pipe(tubs[nProceso]);
    if (!(idHjos[nProceso] = fork())) {
      break;
    }
  }

  if (nProceso == nHijos) {

    for (int i = 0; i < nHijos; i++) {
      close(tubs[i][1]);
    }

    // int cont = 0;
    int buffAux = 0;
    for (int k = 0; k < rondas; k++) {
      for (int i = 0; i < nHijos; i++) {
        read(tubs[i][0], &buffAux, sizeof(int));
      }

      printf("Ronda %d:\n", k+1);
      for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
          matriz[i][j] = matrizAux[i][j];
          printf("[%4d]", matriz[i][j]);
        }
        printf("\n");
      }

      for (int i = 0; i < nHijos; i++) {
        kill(idHjos[i], SIGUSR1);
      }
    }
        for (int i = 0; i < nHijos; i++) {
      close(tubs[i][0]);
    }

  } else {

    for (int i = 0; i < nHijos; i++) {
      if (i == nProceso) {
        close(tubs[i][0]);
      } else {
        close(tubs[i][0]);
        close(tubs[i][1]);
      }
    }

    int nExpuestos = 0, nVerificados = 0;
    float pVerificacion;
    int verficado;
    for (int k = 0; k < rondas; k++) {
      // rondas
      for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
          nExpuestos = 0;
          nVerificados = 0;
          // los que tengo al rededor
          for (int i2 = ((i - 1) < 0 ? 0 : i - 1);
               i2 < ((i + 2) > filas ? filas : i + 2); i2++) {
            for (int j2 = ((j - 1) < 0 ? 0 : j - 1);
                 j2 < ((j + 2) > cols ? cols : j + 2); j2++) {
              if (i == i2 && j == j2) {
                continue;
              }
              if (matriz[i2][j2] == 1 || matriz[i2][j2] == 2) {
                nExpuestos++;
              }
              if (matriz[i2][j2] == 2) {
                nVerificados++;
              }
            }
          }
          if (nProceso == 0) {
            if (matriz[i][j] == 0 && nExpuestos > 1) {
              matrizAux[i][j] = 1;
            } else {
              matrizAux[i][j] = matriz[i][j];
            }
          } else {
            if (matriz[i][j] == 1) {
              pVerificacion = 0.15 + (nVerificados * 0.05);
              verficado = ((float)rand() / RAND_MAX) < pVerificacion;
              matrizAux[i][j] = verficado ? 2 : 1;
            } 
          }
        }
      }
      write(tubs[nProceso][1], &k, sizeof(int));
      pause();
    }
    close(tubs[nProceso][1]);
  }
  return 0;
}
