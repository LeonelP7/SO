#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define EOL '\0'
#define MAX_READ 512

int main(int argc, char const *argv[]) {
  pid_t padre = getpid();
  int nProceso = 0;
  int fd1[3][2][2];

  char buff[MAX_READ];
  int nRead;

  // int fd2[2][2];

  for (int i = 0; i < 2; i++) {
    pipe(fd1[i][0]);
    pipe(fd1[i][1]);
  }

  for (nProceso = 0; nProceso < 2; nProceso++) {
    if (!fork()) {
      pipe(fd1[2][0]);
      pipe(fd1[2][1]);
      if (!fork()) {
        nProceso = 3;
        break;
      }
      break;
    }
  }

  if (nProceso == 2) {
    do {

      for (int i = 0; i < 2; i++) {

        close(fd1[i][0][0]);
        close(fd1[i][1][1]);
      }

      printf("Write 'fin' to exit.\n");
      fgets(buff, MAX_READ, stdin);
      if (strlen(buff) > 1) {
        buff[strlen(buff) - 1] = EOL;
        if (strcmp(buff, "fin") != 0) {

          write(fd1[0][0][1], buff, MAX_READ);
          write(fd1[1][0][1], buff, MAX_READ);
          printf("[%d]writes --> %s\n", getpid(), buff);

          for (int i = 0; i < 2; i++) {
            if ((nRead = read(fd1[i][1][0], buff, MAX_READ)) > 0) {
              buff[nRead] = EOL;
              printf("[%d]reads <-- %s\n", getpid(), buff);
            }
          }
        }
      }
    } while (strcmp(buff, "fin") != 0);

    for (int i = 0; i < 2; i++) {

      close(fd1[i][0][1]);
      close(fd1[i][1][0]);
    }
    for (int i = 0; i < 2; i++) {
      wait(NULL);
    }
  } else {
    if (nProceso < 2) {

      for (int i = 0; i < 2; i++) {
        if (i == nProceso) {
          close(fd1[i][1][0]);
          close(fd1[i][0][1]);
        } else {
          for (int j = 0; j < 2; j++) {
            close(fd1[i][j][0]);
            close(fd1[i][j][1]);
          }
        }
      }

      close(fd1[2][0][0]);
      close(fd1[2][1][1]);

      while ((nRead = read(fd1[nProceso][0][0], buff, MAX_READ))>0) {
        buff[nRead] = EOL;
        printf("[%d][%d]reads <-- %s\n", getpid(), getppid(), buff);
        write(fd1[2][0][1], buff, MAX_READ);
        printf("[%d][%d]write --> %s\n", getpid(), getppid(), buff);

        if ((nRead = read(fd1[2][1][0], buff, MAX_READ))>0) {
          buff[nRead] = EOL;
          printf("[%d][%d]reads <-- %s\n", getpid(), getppid(), buff);
          write(fd1[nProceso][1][1], buff, MAX_READ);
          printf("[%d][%d]write --> %s\n", getpid(), getppid(), buff);
        }
      }

      close(fd1[2][0][1]);
      close(fd1[2][1][0]);
      wait(NULL);

    } else {

      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
          close(fd1[i][j][0]);
          close(fd1[i][j][1]);
        }
      }

      close(fd1[2][0][1]);
      close(fd1[2][1][0]);

      while ((nRead = read(fd1[2][0][0], buff, MAX_READ))>0) {
        buff[nRead] = EOL;
        printf("[%d]reads <-- %s\n", getpid(), buff);
        write(fd1[2][1][1], buff, MAX_READ);
        printf("[%d]write --> %s\n", getpid(), buff);
      }

      close(fd1[2][0][0]);
      close(fd1[2][1][1]);
    }
  }


  return 0;
}
