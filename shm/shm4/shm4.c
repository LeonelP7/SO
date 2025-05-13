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

void rellenarVect(int** v1, int** v2, int* shmid1, int* shmid2, int* tam,
                  char* filename) {
  FILE* fp;
  fp = fopen(filename, "r");
  if (!fp) {
    perror("Error fopen\n");
  }
  fscanf(fp, "%d", tam);

  (*shmid1) = shmget(IPC_PRIVATE, (*tam) * sizeof(int), IPC_CREAT | 0600);
  (*shmid2) = shmget(IPC_PRIVATE, (*tam) * sizeof(int), IPC_CREAT | 0600);
  (*v1) = shmat((*shmid1), NULL, 0);
  (*v2) = shmat((*shmid2), NULL, 0);

  for (int i = 0; i < (*tam) * 2; i++) {

    fscanf(fp, "%d", (i > (*tam) - 1) ? &(*v2)[i - (*tam)] : &(*v1)[i]);
  }
  fclose(fp);
  for (int i = 0; i < (*tam); i++) {
    printf("v1[%d] v2[%d]\n", (*v1)[i], (*v2)[i]);
  }
}

int main(int argc, char const* argv[]) {

  if (argc < 3) {
    printf("Faltan argumentos :(\n");
    exit(1);
  }

  int* v1 = NULL;
  int* v2 = NULL;
  int* resultado;
  int tamaño = 0;

  int nHijos = (int)strtol(argv[1], NULL, 10);
  char* filename = malloc(strlen(argv[2]) + 1);
  strcpy(filename, argv[2]);

  int shmid1 = 0, shmid2 = 0;
  rellenarVect(&v1, &v2, &shmid1,&shmid2,&tamaño, filename);
  int shmid3 = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT | 0600);
  resultado = (int*)shmat(shmid3, NULL, 0);
  *resultado = 0;

  int nProceso;
  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  int delta = (int)ceil((double)tamaño / (nHijos + 1));
  int ini = nProceso * delta;
  int fin = ((nProceso == nHijos) ? tamaño : ini + delta);

  for (int i = ini; i < fin; i++) {
    *resultado += v1[i] * v2[i];
  }

  if (nProceso == nHijos) {
    shmdt(v1);
    shmdt(v2);

    for (size_t i = 0; i < nHijos; i++) {
      wait(NULL);
    }

    printf("Resultado: %d\n",*resultado);
    // for (size_t i = 0; i < tamaño; i++) {
    //   printf("[%d]\n", resultado[i]);
    // }

    shmdt(resultado);
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid3, IPC_RMID, NULL);
  } else {
    shmdt(v1);
    shmdt(v2);
    shmdt(resultado);
  }
  return 0;
}
