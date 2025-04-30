#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_READ 512
#define EOL '\0'

int main(int argc, char const *argv[]) {
  char buff[MAX_READ];

  int fd1[4][2];
  int fd2[2][2];
  int fd3[2][2];

  int nRead;

  for (int i = 0; i < 4; i++) {
    pipe(fd1[i]);
  }

  int nProceso = 2;
  int nivel = 0;

  for (nProceso = 2; nProceso >= 0; nProceso--) {

    if (!fork()) {

      nivel = 1;
      pipe(fd2[0]);
      pipe(fd2[1]);
      if (!fork()) {

        nivel = 2;

        if (nProceso != 1) {
          pipe(fd3[0]);
          pipe(fd3[1]);
          if (!fork()) {

            nivel = 3;

            break;
          }
        }
        break;
      }
      break;
    }
  }

  if (nivel == 0) {

    for (int i = 0; i < 4; i++) {
      if (i == 0) {
        close(fd1[i][0]);
      } else if (i == 3) {
        close(fd1[i][1]);
      } else {
        close(fd1[i][0]);
        close(fd1[i][1]);
      }
    }


    do {
      printf("Write 'fin' to exit.\n");
      fgets(buff, MAX_READ, stdin);

      if (strlen(buff) > 1) {
        buff[strlen(buff) - 1] = EOL;

        // write(fd1[0][1], buff, MAX_READ);
        //   printf("[%d]writes --> %s\n", getpid(), buff);

        if (strcmp(buff, "fin") != 0) {
          write(fd1[0][1], buff, MAX_READ);
          printf("[%d]writes --> %s\n", getpid(), buff);

          if ((nRead = read(fd1[3][0], buff, MAX_READ)) > 0) {
            printf("[%d]reads <-- %s\n", getpid(), buff);
          }
        } else {
          printf("End.\n");
        }
      }

    } while (strcmp(buff, "fin") != 0);

    close(fd1[0][1]);
    close(fd1[3][0]);
    for (int i = 0; i < 2; i++) {
      wait(NULL);
    }

  } else {
    if (nivel == 1) {

      for (int i = 0; i < 4; i++) {
        if (i == nProceso) {
          close(fd1[i][1]);
        } else if (i == nProceso + 1) {
          close(fd1[i][0]);
        } else {
          close(fd1[i][0]);
          close(fd1[i][1]);
        }
      }

      close(fd2[0][0]);
      close(fd2[1][1]);

      while ((nRead = read(fd1[nProceso][0], buff, MAX_READ)) > 0) {
        buff[nRead] = EOL;
        printf("[%d][%d]reads <-- %s\n", getpid(), getppid(), buff);
        printf("[%d]writes --> %s\n", getpid(), buff);
        write(fd2[0][1], buff, MAX_READ);

        if ((nRead = read(fd2[1][0], buff, MAX_READ)) > 0) {
          buff[nRead] = EOL;
          printf("[%d]reads <-- %s\n", getpid(), buff);
          printf("[%d]writes --> %s\n", getpid(), buff);
          write(fd1[nProceso + 1][1], buff, MAX_READ);
        }
      }

      close(fd2[0][1]);
      close(fd2[1][0]);

      wait(NULL);

    } else if (nivel == 2) {
      for (int i = 0; i < 4; i++) {
        close(fd1[i][0]);
        close(fd1[i][1]);
      }


      for (int i = 0; i < 2; i++) {
        close(fd2[i][i ? 0 : 1]);
        if (nProceso != 1) {
          close(fd3[i][i]);
        }
      }
      /*
      el for de arriba hace esto
      close(fd2[0][1]);
      close(fd2[1][0]);
      close(fd3[0][0]);
      close(fd3[1][1]);
      */

      while ((nRead = read(fd2[0][0], buff, MAX_READ)) > 0) {
        buff[nRead] = EOL;
        printf("[%d]reads <-- %s\n", getpid(), buff);
        printf("[%d]writes --> %s\n", getpid(), buff);

        if (nProceso == 1) {
          write(fd2[1][1], buff, MAX_READ);
        } else {
          write(fd3[0][1], buff, MAX_READ);

          if ((nRead = read(fd3[1][0], buff, MAX_READ)) > 0) {
            buff[nRead] = EOL;
            printf("[%d]reads <-- %s\n", getpid(), buff);
            printf("[%d]writes --> %s\n", getpid(), buff);
            write(fd2[1][1], buff, MAX_READ);
          }
        }
      }

      for (int i = 0; i < 2; i++) {
        close(fd2[i][i]);
        if (nProceso != 1) {
          close(fd3[i][i ? 1 : 0]);
        }
      }

      /*
      el for de arriba hace esto
      close(fd3[0][1]);
      close(fd3[1][0]);
      close(fd2[0][0]);
      close(fd2[1][1]);
      */
      wait(NULL);
    } else {

      for (int i = 0; i < 4; i++) {
        close(fd1[i][0]);
        close(fd1[i][1]);
        if (i == 0 || i == 1) {
          close(fd2[i][0]);
          close(fd2[i][1]);
        }
      }

      close(fd3[0][1]);
      close(fd3[1][0]);

      while ((nRead = read(fd3[0][0], buff, MAX_READ)) > 0) {
        buff[nRead] = EOL;
        printf("[%d]reads <-- %s\n", getpid(), buff);
        printf("[%d]writes --> %s\n", getpid(), buff);
        write(fd3[1][1], buff, MAX_READ);
      }

      close(fd3[0][0]);
      close(fd3[1][1]);
    }
  }
  return EXIT_SUCCESS;
}
