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

void error(char *msg) {
  perror(msg);
  exit(-1);
}

int idshm;

int leerChar(char *filename, char **vec) {
  int c, totalChar;
  char caracter;
  FILE *infile;
  infile = fopen(filename, "r");
  if (!infile) {
    error("Error fopen\n");
  }
  fscanf(infile, "%d", &totalChar);
  idshm = shmget(IPC_PRIVATE, totalChar * sizeof(char), IPC_CREAT | 0600);
  *vec = shmat(idshm, NULL, 0);
  if (!*vec) {
    error("error calloc");
  }
  for (c = 0; c < totalChar; c++) {
    fscanf(infile, "%c", &caracter);
    (*vec)[c] = caracter;
    // printf("%c\n", (*vec)[c]);
  }
  fclose(infile);
  return c;
}

struct Estadistica {
  char letra;
  int cantidad;
};


int main(int argc, char const *argv[]) {

  if (argc < 3) {
    return 1;
  }

  char *vec;
  char fileName[100];
  strcpy(fileName, argv[2]);
  char *endptr;
  int nHijos = strtol(argv[1], &endptr, 10);
  int buff;

  int nLeido = leerChar(fileName, &vec);
  int nProceso;

  int fd[nHijos][2];

  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    pipe(fd[nProceso]);
    if (!fork()) {
      break;
    }
  }

  if (nProceso == nHijos) {

    printf("%s\n", vec);

    for (int i = 0; i < nHijos; i++) {
      close(fd[i][1]);
    }

    int nRead;
    struct Estadistica buffE;
    printf("Estadisticas:\n");
    while ((nRead = read(fd[nHijos - 1][0], &buffE,
                         sizeof(struct Estadistica))) > 0) {
      printf("%c: %d\n", buffE.letra, buffE.cantidad);
    }


    for (int i = 0; i < nHijos - 1; i++) {
      printf("\n");
      printf("Proceso [%d]:\n", i);
      while ((nRead = read(fd[i][0], &buff, sizeof(int))) > 0) {
        printf("Secuencia en la posicion: %d\n", buff);
      }
    }

    for (int i = 0; i < nHijos; i++) {
      close(fd[i][0]);
    }


    for (int i = 0; i < nHijos; i++) {
      wait(NULL);
    }

    shmdt(vec);
    shmctl(idshm, IPC_RMID, NULL);


  } else if (nProceso == nHijos - 1) {  // el que cuenta

    for (int i = 0; i < nHijos; i++) {
      if (i == nProceso) {
        close(fd[i][0]);
      } else {
        close(fd[i][0]);
        close(fd[i][1]);
      }
    }

    struct Estadistica eA;
    struct Estadistica eT;
    struct Estadistica eC;
    struct Estadistica eG;

    eA.cantidad = 0;
    eA.letra = 'A';
    eT.cantidad = 0;
    eT.letra = 'T';
    eC.cantidad = 0;
    eC.letra = 'C';
    eG.cantidad = 0;
    eG.letra = 'G';

    char letra;

    for (int i = 0; i < nLeido; i++) {
      letra = vec[i];
      switch (letra) {
        case 'A':
          eA.cantidad++;
          break;
        case 'T':
          eT.cantidad++;
          break;
        case 'C':
          eC.cantidad++;
          break;
        case 'G':
          eG.cantidad++;
          break;
      }
    }

    write(fd[nProceso][1], &eA, sizeof(struct Estadistica));
    write(fd[nProceso][1], &eT, sizeof(struct Estadistica));
    write(fd[nProceso][1], &eC, sizeof(struct Estadistica));
    write(fd[nProceso][1], &eG, sizeof(struct Estadistica));


    close(fd[nProceso][1]);
    shmdt(vec);


  } else {  // los que buscan

    for (int i = 0; i < nHijos; i++) {
      if (i == nProceso) {
        close(fd[i][0]);
      } else {
        close(fd[i][0]);
        close(fd[i][1]);
      }
    }
    
    int delta = nLeido / (nHijos-1);
    int ini = delta * nProceso;
    int fin = ini + delta;
    char secuencia[7];

    for (int i = ini; i < fin; i++) {
      strncpy(secuencia, vec + i, 6);
      secuencia[7] = '\0';
      if (strcmp(secuencia, "GCGTGA") == 0) {
        //printf("encontre\n");
        write(fd[nProceso][1], &i, sizeof(int));
      } else {
        // printf("no encontre \n");
      }
    }
  }

  close(fd[nProceso][1]);
  shmdt(vec);

  return 0;
}
