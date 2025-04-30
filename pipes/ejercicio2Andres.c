#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define EOL '\0'

int main() {

  int i;
  pid_t root = getpid();
  char buffer[100];

  int fd1[4][2];
  int fd2[2][2];
  int nRead;

  int nivel = 0;
  for (int j = 0; j < 4; j++) {
    pipe(fd1[j]);
  }

  for (i = 0; i < 2; i++) {
    if (!fork()) {
      nivel = 1;
      pipe(fd2[0]);
      pipe(fd2[1]);

      if (!fork()) {
        nivel = 2;
        break;
      }
      break;
    }
  }

  if (nivel == 0) {
    for (int j = 0; j < 4; j++) {
      if (j % 2 == 0) {
        close(fd1[j][0]);
      } else {
        close(fd1[j][1]);
      }
    }

    do {

      printf("Escriba 'fin' para terminar.\n");
      fgets(buffer, sizeof(buffer), stdin);

      if (strlen(buffer) > 1) {

        buffer[strlen(buffer) - 1] = EOL;

        write(fd1[0][1], buffer, sizeof(buffer));
        write(fd1[2][1], buffer, sizeof(buffer));
        printf("Proceso [%d] escribe: %s\n", getpid(), buffer);
      }


      if ((nRead = read(fd1[1][0], buffer, sizeof(buffer))) > 0) {
        printf("fokin\n");
        buffer[nRead] = EOL;
        printf("Proceso [%d] lee: %s\n", getpid(), buffer);
      }
      if ((nRead = read(fd1[3][0], buffer, sizeof(buffer))) > 0) {
        buffer[nRead] = EOL;
        printf("Proceso [%d] lee: %s\n", getpid(), buffer);
      }

    } while (strcmp(buffer, "fin") != 0);

    for (int j = 0; j < 4; i++) {
      if (j % 2 != 0) {
        close(fd1[j][0]);
      } else {
        close(fd1[j][1]);
      }
    }

    for (int j = 0; j < 4; j++) {
      wait(NULL);
    }

  } else if (nivel == 1) {

    for (int j = 0; j < 4; j++) {
      if (i % 2 != 0) {
        close(fd1[j][0]);
      } else {
        close(fd1[j][1]);
      }
    }

    close(fd2[0][0]);
    close(fd2[1][1]);

    while ((nRead = read(fd1[i * 2][0], buffer, sizeof(buffer))) > 0) {
      buffer[nRead] = EOL;
      printf("Proceso [%d] lee: %s papa: %d\n", getpid(), buffer, getppid());
      write(fd2[0][1], buffer, sizeof(buffer));
      printf("Proceso [%d] escribe: %s papá: %d\n", getpid(), buffer,
             getppid());

      if ((nRead = read(fd2[1][0], buffer, sizeof(buffer))) > 0) {
        buffer[nRead] = EOL;
        printf("Proceso [%d] lee: %s papá: %d\n", getpid(), buffer, getppid());
        write(fd1[(i * 2) + 1][1], buffer, sizeof(buffer));
        printf("Proceso [%d] escribe: %s papá: %d\n", getpid(), buffer,
               getppid());
      }
    }

    for (int j = 0; j < 4; j++) {
      if (i % 2 == 0) {
        close(fd1[j][0]);
      } else {
        close(fd1[j][0]);
        close(fd1[j][1]);
      }
    }

    close(fd2[0][1]);
    close(fd2[1][0]);

  } else {

    for (int j = 0; j < 4; j++) {
      close(fd1[j][0]);
      close(fd1[j][1]);

      if (j == 0) {
        close(fd2[j][1]);
      }
      if (j == 1) {
        close(fd2[j][0]);
      }
    }

    while ((nRead = read(fd2[0][0], buffer, sizeof(buffer))) > 0) {
      buffer[nRead] = EOL;
      printf("Proceso [%d] lee: %s papá: %d\n", getpid(), buffer, getppid());
      write(fd2[1][1], buffer, sizeof(buffer));
      printf("Proceso [%d] escribe: %s papá: %d\n", getpid(), buffer,
             getppid());
    }

    close(fd2[0][0]);
    close(fd2[1][1]);
  }

  return 0;
}