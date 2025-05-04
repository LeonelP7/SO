#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct DatosMatriz {
  int x, y;
  int valor;
};

int main(int argc, char const *argv[]) {

  int nProceso;
  int rows = 3, cols = 3;
  int fd[3][2];

  int **matrizA, **matrizB, **matrizC;

  matrizA = (int **)malloc(rows * sizeof(int *));
  matrizB = (int **)malloc(rows * sizeof(int *));
  matrizC = (int **)malloc(rows * sizeof(int *));

  for (int i = 0; i < rows; i++) {
    matrizA[i] = (int *)malloc(cols * sizeof(int));
    matrizB[i] = (int *)malloc(cols * sizeof(int));
    matrizC[i] = (int *)malloc(cols * sizeof(int));

    for (int j = 0; j < cols; j++) {
      matrizA[i][j] = (i * cols) + j;
      matrizB[i][j] = ((i * cols) + j) * 2;
    }
  }

  for (nProceso = 0; nProceso < 3; nProceso++) {
    pipe(fd[nProceso]);
    if (!fork()) {
      break;
    }
  }

  if (nProceso == 3) {
    int nRead;
    struct DatosMatriz datosC;

    for (int i = 0; i < 3; i++) {
      close(fd[i][1]);
    }

    for (int i = 0; i < nProceso; i++) {
        wait(NULL);
    }

    for (int i = 0; i < 3; i++)
    {
        while (nRead = read(fd[i][0], &datosC, sizeof(struct DatosMatriz)) > 0)
        {
            matrizC[datosC.x][datosC.y] = datosC.valor;
        }
        
    }

    printf("Matriz C:\n");
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        printf("[%d]", matrizC[i][j]);
      }
      printf("\n");
    }

    for (int i = 0; i < 3; i++) {
      close(fd[i][0]);
    }
    
  } else if (nProceso == 0) {

    close(fd[0][0]);

    struct DatosMatriz datos;

    for (int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++){
            if (i<j)
            {
                datos.valor = 0;
                datos.x = i;
                datos.y = j;
                for (int k = 0; k < cols; k++)
                {
                    datos.valor += matrizA[i][k]*matrizB[k][j];
                }
                write(fd[0][1],&datos,sizeof(struct DatosMatriz));
            }
        }
    }

    close(fd[0][1]);
  }else if (nProceso == 1)
  {
    close(fd[0][0]);
    close(fd[0][1]);
    close(fd[1][0]);

    struct DatosMatriz datos;

    for (int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++){
            if (i==j)
            {
                datos.valor = 0;
                datos.x = i;
                datos.y = j;
                for (int k = 0; k < cols; k++)
                {
                    datos.valor += matrizA[i][k]*matrizB[k][j];
                }
                write(fd[1][1],&datos,sizeof(struct DatosMatriz));
            }
        }
    }

    close(fd[1][1]);
    
  }else{
    for (int i = 0; i < 3; i++)
    {
        if (i == 2)
        {
            close(fd[i][0]);
        }else{
            close(fd[i][0]);
            close(fd[i][1]);
        }
    }
    

    struct DatosMatriz datos;

    for (int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++){
            if (i>j)
            {
                datos.valor = 0;
                datos.x = i;
                datos.y = j;
                for (int k = 0; k < cols; k++)
                {
                    datos.valor += matrizA[i][k]*matrizB[k][j];
                }
                write(fd[2][1],&datos,sizeof(struct DatosMatriz));
            }
        }
    }

    close(fd[2][1]);
  }
  


  return EXIT_SUCCESS;
}
