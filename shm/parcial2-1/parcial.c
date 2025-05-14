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

struct Datos {
  char *palabra;
  char *sentido;
  int i;
  int j;
};


int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Faltan argumentos :(\n");
  }

  FILE *f = fopen(argv[1], "r");
  int rows = 0, cols = 0;
  fscanf(f, "%d", &rows);
  fscanf(f, "%d", &cols);

  // matriz de letras
  int idMatriz = shmget(IPC_PRIVATE, sizeof_dm(rows, cols, sizeof(char)),
                        IPC_CREAT | 0600);
  char **matriz = shmat(idMatriz, NULL, 0);
  create_index((void *)matriz, rows, cols, sizeof(char));

  printf("Matriz:\n");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fscanf(f, " %c", &matriz[i][j]);
      printf("[%2c]", matriz[i][j]);
    }
    printf("\n");
  }

  int nPalabras;
  fscanf(f, "%d", &nPalabras);
  // vector de palabras
  char vec[nPalabras][20];
  int fd[nPalabras][2];
  int nProceso;
  for (nProceso = 0; nProceso < nPalabras; nProceso++) {
    fscanf(f, "%s", vec[nProceso]);
    pipe(fd[nProceso]);
    if (!fork()) {
      break;
    }
  }
  fclose(f);

  if (nProceso == nPalabras) {
    shmdt(matriz);
    for (int i = 0; i < nPalabras; i++) {
      close(fd[i][1]);
    }

    struct Datos datos = {
        .palabra = (char *)calloc(strlen(vec[nProceso]) + 1, sizeof(char)),
        .sentido = (char *)calloc(100, sizeof(char)),
        .i = 0,
        .j = 0};

    for (int i = 0; i < nPalabras; i++) {
        printf("Proceso %d\n",i);
      while ((read(fd[i][0],&datos,sizeof(struct Datos)))>0) {
        printf("i: %d j: %d sentido: %s palabra: %s\n",datos.i,datos.j,datos.sentido,datos.palabra);
      }
      close(fd[i][0]);
    }

    for (int i = 0; i < nPalabras; i++)
    {
        wait(NULL);
    }
    shmctl(idMatriz,IPC_RMID,NULL);
  } else {
    for (int i = 0; i < nProceso + 1; i++) {
      if (i == nProceso) {
        close(fd[i][0]);
      } else {
        close(fd[i][0]);
        close(fd[i][1]);
      }
    }

    struct Datos datos = {
        .palabra = (char *)calloc(strlen(vec[nProceso]) + 1, sizeof(char)),
        .sentido = (char *)calloc(100, sizeof(char)),
        .i = 0,
        .j = 0};

    char *palabraAux = (char *)calloc(strlen(vec[nProceso]) + 1, sizeof(char));

    // printf("%s\n",derecha);
    int p = 0;
    int fin = 0;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        // izquierda a derecha
        fin = j + strlen(vec[nProceso]);
        for (int j2 = j; j2 < ((fin > cols) ? cols : fin); j2++) {
          // printf("hola\n");
          palabraAux[p] = matriz[i][j2];
          datos.i = i;
          datos.j = j2;
          p++;
        }
        palabraAux[p] = '\0';
        p = 0;
        if (strcmp(vec[nProceso], palabraAux) == 0) {
          // printf("Lo encontre :)\n");
          datos.palabra = palabraAux;
          datos.sentido = "izquierda a derecha";
          write(fd[nProceso][1], &datos, sizeof(struct Datos));
        }

        // arriba hacia abajo
        fin = i + strlen(vec[nProceso]);
        for (int j2 = i; j2 < ((fin > rows) ? rows : fin); j2++) {
          // printf("hola\n");
          palabraAux[p] = matriz[j2][j];
          datos.i = i;
          datos.j = j2;
          p++;
        }
        palabraAux[p] = '\0';
        p = 0;
        if (strcmp(vec[nProceso], palabraAux) == 0) {
          datos.palabra = palabraAux;
          datos.sentido = "Arriba hacia abajo";
          write(fd[nProceso][1], &datos, sizeof(struct Datos));
        }

        // derecha a izquierda
        fin = j + strlen(vec[nProceso]);
        for (int j2 = ((fin > cols) ? cols : fin); j2 > j; j2--) {
          // printf("hola\n");
          palabraAux[p] = matriz[i][j2];
          datos.i = i;
          datos.j = j2;
          p++;
        }
        palabraAux[p] = '\0';
        p = 0;
        if (strcmp(vec[nProceso], palabraAux) == 0) {
          datos.palabra = palabraAux;
          datos.sentido = "derecha a izquierda";
          write(fd[nProceso][1], &datos, sizeof(struct Datos));
        }

        // abajo hacia arriba
        fin = i + strlen(vec[nProceso]);
        for (int j2 = ((fin > cols) ? cols : fin); j2 > i; j2--) {
          // printf("hola\n");
          palabraAux[p] = matriz[j2][j];
          datos.i = i;
          datos.j = j2;
          p++;
        }
        palabraAux[p] = '\0';
        p = 0;
        if (strcmp(vec[nProceso], palabraAux) == 0) {
          datos.palabra = palabraAux;
          datos.sentido = "derecha a izquierda";
          write(fd[nProceso][1], &datos, sizeof(struct Datos));
        }
      }
    }

    close(fd[nProceso][1]);
    shmdt(matriz);
  }


  return 0;
}
