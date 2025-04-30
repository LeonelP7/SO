#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_READ 256  // Tamaño máximo del buffer de lectura.
#define EOL '\0'

int main(int argc, char const *argv[]) {
  int fd[4][2];            // Descriptores de archivo para los pipes.
  char buffIn[MAX_READ];   // Buffer para escribir los mensajes de entrada.
  char buffOut[MAX_READ];  // Buffer para leer los mensajes desde los hijos.

  for (int ia = 0; ia < 4; ia++) {
    pipe(fd[ia]);
  }

  int nProceso = 4;  // indice de los hijos
  for (int i = 0; i < 2; i++) {
    if (!fork()) {
      if (i == 0) {
        nProceso = 0;
      } else {
        nProceso = 2;
      }
      if (!fork()) {
        if (i == 0) {
          nProceso = 1;
        } else {
          nProceso = 3;
        }
        break;
      }
      break;
    }
  }

  if (nProceso == 4)  // proceso padre
  {
    close(fd[0][0]);  // cierra el lectura de para el pipe que conecta con hijo1
    // cierra las pipes que no usa
    close(fd[1][0]);
    close(fd[1][1]);
    close(fd[2][0]);
    close(fd[2][1]);
    close(fd[3][0]);
    close(fd[3][1]);
    printf("Write 'fin' to exit.\n\n");
    do {
      fgets(buffIn, MAX_READ, stdin);

      if (strlen(buffIn) > 1) {
        buffIn[strlen(buffIn) - 1] = EOL;
        if (strcmp(buffIn, "fin") != 0) {
          printf("[%d]write -->: %s\n", getpid(), buffIn);
          write(fd[0][1], buffIn, MAX_READ);
        } else {
          printf("end.\n");
        }
      }
    } while (strcmp(buffIn, "fin") != 0);

    close(fd[0][1]);
    for (int i = 0; i < 4; i++) {
      wait(NULL);
    }
  } else {
    int n;
    for (int ib = 0; ib < 4; ib++) {
      if (nProceso == ib) {
        close(fd[ib][1]);  // cierro escritura de la que leo
      } else if ((nProceso + 1) == ib) {
        close(fd[ib][0]);  // cierro lectura de la escribo
      } else               // cierro todas las demas
      {
        close(fd[ib][0]);
        close(fd[ib][1]);
      }
    }
    while ((n = read(fd[nProceso][0], buffOut, MAX_READ)) > 0) {
      buffOut[n] = EOL;
      printf("[%d]read <--: %s\n", getpid(), buffOut);
      if (nProceso != 3) {
        printf("[%d]write -->: %s\n", getpid(), buffOut);
        write(fd[nProceso + 1][1], buffOut, MAX_READ);
      }
    }
    close(fd[nProceso][0]);
    if (nProceso != 3) {
      close(fd[nProceso + 1][1]);
    }
  }

  return EXIT_SUCCESS;
}
