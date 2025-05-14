#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct Transaccion {
  char linea[256];
  float cantidad;
};

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Faltan args :(\n");
    exit(1);
  }

  FILE *f = fopen(argv[1], "r");
  int nTransacciones = 0;
  fscanf(f, "%d", &nTransacciones);

  int nHijos = 3;
  int nProceso;
  int fd[nHijos + 1][2];
  for (int i = 0; i < nHijos + 1; i++) {
    pipe(fd[i]);
  }
  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    if (!fork()) {
      break;
    }
  }

  if (nProceso == nHijos) {

    for (int i = 0; i < nHijos + 1; i++) {
      if (i == 0) {
        close(fd[i][0]);
      } else if (i == nHijos) {
        close(fd[i][1]);
      } else {
        close(fd[i][0]);
        close(fd[i][1]);
      }
    }

    char buff[256];
    regex_t regex;
    const char *patron =
        "^[a-zA-Z0-9]+: [a-zA-Z0-9]+ -> [a-zA-Z0-9]+ : ([0-9]+(\\.[0-9]+)?)$";
    regmatch_t matches[2];
    char numStr[50];
    float num = 0;

    if (regcomp(&regex, patron, REG_EXTENDED) != 0) {
      fprintf(stderr, "No se pudo compilar la expresion regular\n");
      return 1;
    }

    struct Transaccion buffOut = {.cantidad = 0};

    while (fgets(buff, sizeof(buff), f)) {
      buff[strcspn(buff, "\n")] = 0;
      if (regexec(&regex, buff, 2, matches, 0) == 0) {
        strncpy(numStr, &buff[matches[1].rm_so],
                matches[1].rm_eo - matches[1].rm_so);
        numStr[matches[1].rm_eo - matches[1].rm_so] = '\0';

        num = strtof(numStr, NULL);
        strcpy(buffOut.linea, buff);
        buffOut.cantidad = num;
        // printf("Línea válida: %s\n", buffOut.linea);
        // printf("Cantidad: %.2f\n", buffOut.cantidad);

        write(fd[0][1], &buffOut, sizeof(struct Transaccion));
      }
    }
    fclose(f);
    close(fd[0][1]);

    printf("Transacciones validas:\n");
    while (read(fd[nHijos][0], &buffOut, sizeof(struct Transaccion)) > 0) {
      printf("%s\n", buffOut.linea);
    }
    close(fd[nHijos][0]);

    for (int i = 0; i < nHijos; i++) {
      wait(NULL);
    }

  } else {  // hijos
    for (int i = 0; i < nHijos + 1; i++) {
      if (i == nProceso) {
        close(fd[i][1]);
      } else if (i == nProceso + 1) {
        close(fd[i][0]);
      } else {
        close(fd[i][0]);
        close(fd[i][1]);
      }
    }

    struct Transaccion buffIn = {.cantidad = 0};

    while (read(fd[nProceso][0], &buffIn, sizeof(struct Transaccion)) > 0) {
      switch (nProceso) {
        case 0:
          if (strstr(buffIn.linea, "TRX:") != NULL) {
            write(fd[nProceso + 1][1], &buffIn, sizeof(struct Transaccion));
          }
          break;
        case 1:
          if(buffIn.cantidad > 0 && buffIn.cantidad <= 10000){
            write(fd[nProceso + 1][1], &buffIn, sizeof(struct Transaccion));
          }
          break;
        case 2:
        if (strstr(buffIn.linea, "hacker") == NULL &&
              strstr(buffIn.linea, "vault") == NULL &&
              strstr(buffIn.linea, "cashout") == NULL) {
            write(fd[nProceso + 1][1], &buffIn, sizeof(struct Transaccion));
          }
          break;
      }
    }
    close(fd[nProceso][0]);
    close(fd[nProceso + 1][1]);
  }
  return 0;
}
