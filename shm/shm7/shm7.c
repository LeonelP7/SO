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

int contador = 0;
int padreMandando = 0;

void manejador(int sig) {
  if (!padreMandando) {
    contador++;
  }
}

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

  int dias, hijos, rows, cols;
  fscanf(f, "%d", &dias);
  fscanf(f, "%d", &hijos);
  fscanf(f, "%d", &rows);
  fscanf(f, "%d", &cols);

  int idShm =
      shmget(IPC_PRIVATE, sizeof_dm(rows, cols, sizeof(int)), IPC_CREAT | 0600);
  int **matriz = shmat(idShm, NULL, 0);
  create_index((void *)matriz, rows, cols, sizeof(int));

  int idShm2 =
      shmget(IPC_PRIVATE, sizeof_dm(rows, cols, sizeof(int)), IPC_CREAT | 0600);
  int **matrizAux = shmat(idShm2, NULL, 0);
  create_index((void *)matrizAux, rows, cols, sizeof(int));

  printf("Matriz:\n");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fscanf(f, "%d", &matriz[i][j]);
      printf("[%4d]", matriz[i][j]);
    }
    printf("\n");
  }

  signal(SIGUSR1,manejador);

  int nProceso;
  int *idHijos = calloc(hijos, sizeof(pid_t));
  for (nProceso = 0; nProceso < hijos; nProceso++) {
    if (!(idHijos[nProceso] = fork())) {
      break;
    }
  }

  if (nProceso == hijos) {


    for (int k = 0; k < dias; k++) {
      while (contador < hijos) {
        pause();
      }

      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          matriz[i][j] = matrizAux[i][j];
        }
      }

      padreMandando = 1;
      for (int i = 0; i < hijos; i++) {
        kill(idHijos[i], SIGUSR1);
      }
      padreMandando = 0;
    }

    for (int i = 0; i < hijos; i++)
    {
        wait(NULL);
    }
    shmdt(matrizAux);
    printf("Resultado:\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("[%4d]",matriz[i][j]);
        }
        printf("\n");
    }
    shmdt(matriz);
    shmctl(idShm,IPC_RMID,NULL);
    shmctl(idShm2,IPC_RMID,NULL);
  } else {
    int delta = (int)ceil((float)rows / hijos);
    int ini = delta * nProceso;
    if (ini >= rows) {
      exit(1);
    }
    int fin = (ini + delta) > rows ? rows : ini + delta;

    int vecInfectado = 0, vecRecuperados = 0;
    float pRecuperacion = 0;
    int recuperado = 0;
    for (int diasP = 0; diasP < dias; diasP++) {
      for (int i = ini; i < fin; i++) {
        vecInfectado = 0;
        vecRecuperados = 0;
        for (int j = 0; j < cols; j++) {

          // estos fors son lo que tiene al rededor m[i][j]
          for (int i2 = ((i - 1) < 0 ? 0 : i - 1);
               i2 < ((i + 2) >= rows ? rows : i + 2); i2++) {
            for (int j2 = ((j - 1) < 0 ? 0 : j - 1);
                 j2 < ((j + 2) >= cols ? cols : j + 2); j2++) {

              if (i == i2 && j2 == j) {
                continue;
              } else if (matriz[i2][j2] == 1) {
                vecInfectado++;
              } else if (matriz[i2][j2] == 2) {
                vecRecuperados++;
              }
            }
          }

          if (matriz[i][j] == 0 && vecInfectado > 1) {
            matrizAux[i][j] = 1;
          } else if (matriz[i][j] == 1) {
            pRecuperacion = 0.2 + (vecRecuperados * 0.05);
            recuperado = ((float)rand() / RAND_MAX) < pRecuperacion;
            if (recuperado) {
              matrizAux[i][j] = 2;
            }else{
                matrizAux[i][j] = 1;
            }
          }else {
            matrizAux[i][j] = matriz[i][j];
          }
        }
      }
      kill(getppid(), SIGUSR1);
      pause();
    }
    shmdt(matriz);
    shmdt(matrizAux);
  }


  return 0;
}
