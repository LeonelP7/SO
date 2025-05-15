#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Faltan args\n");
    exit(1);
  }

  FILE *f = fopen(argv[1], "r");

  int nTransacciones = 0;
  fscanf(f, "%d", &nTransacciones);
  fgetc(f);

  int nHijos = 2;
  int nProceso = 0;
  int fd[5][2];
  char buff[256];
  for (int i = 0; i < 5; i++) {
    pipe(fd[i]);
  }

  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  if (nProceso == nHijos) {

    for (int i = 0; i < 5; i++) {
      if (i == 0) {
        close(fd[i][0]);
      } else if (i == 2 || i == 3 || i == 4) {
        close(fd[i][1]);
      } else {
        close(fd[i][0]);
        close(fd[i][1]);
      }
    }

    while (fgets(buff, sizeof(buff), f)) {
      buff[strcspn(buff, "\n")] = 0;
      // printf("%s\n",buff);
      write(fd[0][1], &buff, sizeof(buff));
    }
    fclose(f);
    close(fd[0][1]);

    char *prioridad[] = {"BAJA PRIORIDAD", "URGENTES", "CRÍTICAS"};
    for (int i = 4; i >= 2; i--) {
      printf("%s\n", prioridad[i - 2]);
      while (read(fd[i][0], &buff, sizeof(buff)) > 0) {
        printf("%s\n", buff);
      }
    }
    for (int i = 2; i <= 4; i++) {
      close(fd[i][0]);
    }
    for (int i = 0; i < nHijos; i++) {
      wait(NULL);
    }
  } else {
    for (int i = 0; i < 5; i++) {
      if (i == nProceso) {
        close(fd[i][1]);
      }
      if (nProceso == 0) {
        if (i == 1 || i == 2) {
          close(fd[i][0]);
        } else if (i != 0) {
          close(fd[i][0]);
          close(fd[i][1]);
        }
      }
      if (nProceso == 1) {
        if (i == 4 || i == 3) {
          close(fd[i][0]);
        } else if (i != 1) {
          close(fd[i][0]);
          close(fd[i][1]);
        }
      }
    }


    while (read(fd[nProceso][0], buff, sizeof(buff)) > 0) {
      // printf("%s\n",buff);
      switch (nProceso) {
        case 0:
          if (strncmp(buff, "REQ:", sizeof("REQ:")-1) == 0 &&
              strstr(buff, ";") != NULL && strstr(buff, "URGENTE") != NULL) {
            write(fd[nProceso + 1][1], buff, sizeof(buff));
          } else {
            //printf("baja prioridad %s\n", buff);
            write(fd[2][1], buff, sizeof(buff));
          }
          break;
        case 1:
          if (strstr(buff, "servidor") != NULL ||
              strstr(buff, "bloqueo") != NULL ||
              strstr(buff, "caida") != NULL) {
            write(fd[4][1], buff, sizeof(buff));
            // printf("hola\n");
          } else {
            write(fd[3][1], buff, sizeof(buff));
          }
          break;
      }
    }

    close(fd[nProceso][0]);
    switch (nProceso) {
      case 0:
        close(fd[nProceso + 1][1]);
        close(fd[2][1]);
        break;

      case 1:
        close(fd[4][1]);
        close(fd[3][1]);
        break;
    }
  }


  return 0;
}
