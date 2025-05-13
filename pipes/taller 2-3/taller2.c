#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct Job {
  int jobId;
  float kwh;
  int renovable;
};

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Faltan argumentos :(\n");
  }
  FILE *f = fopen(argv[1], "r");
  int nJobs = 0;
  fscanf(f, "%d", &nJobs);

  int nHijos = 2;
  int nProceso = 0;

  int fd[nHijos][2][2];

  for (nProceso = 0; nProceso < nHijos; nProceso++) {
    pipe(fd[nProceso][0]);
    pipe(fd[nProceso][1]);
    if (!fork()) {
      break;
    }
  }

  if (nProceso == nHijos) {

    for (int i = 0; i < nHijos; i++) {
      close(fd[i][1][1]);
      close(fd[i][0][0]);
    }

    struct Job job;
    float buffR;
    for (int i = 0; i < nJobs; i++) {
      fscanf(f, "%d;%f;%d", &job.jobId, &job.kwh, &job.renovable);
      if (!job.renovable) {
        write(fd[0][0][1], &job, sizeof(struct Job));
        write(fd[1][0][1], &job, sizeof(struct Job));
      } else {
        write(fd[1][0][1], &job, sizeof(struct Job));
      }
    }

    for (int i = 0; i < nHijos; i++) {
      close(fd[i][0][1]);
    }

    printf("Resultados:\n");
    read(fd[0][1][0], &buffR, sizeof(float));
    printf("Emision total: %.2f\n", buffR);

    read(fd[1][1][0], &buffR, sizeof(float));
    printf("Ahorro total: %.2f\n", buffR);

    for (int i = 0; i < nHijos; i++) {
      close(fd[i][1][0]);
    }
    for (int i = 0; i < nHijos; i++) {
      wait(NULL);
    }

  } else {

    for (int i = 0; i < nHijos; i++) {
      if (i == nProceso) {
        close(fd[i][1][0]);
        close(fd[i][0][1]);
      } else {
        close(fd[i][0][0]);
        close(fd[i][1][1]);
        close(fd[i][1][0]);
        close(fd[i][0][1]);
      }
    }

    float resultado = 0;
    struct Job buff = {0,0,0};
    int nRead;
    while ((nRead = read(fd[nProceso][0][0], &buff, sizeof(struct Job))) > 0) {
      resultado += buff.kwh;
    }
    resultado = resultado * (0.4);
    write(fd[nProceso][1][1], &resultado, sizeof(float));

    close(fd[nProceso][0][0]);
    close(fd[nProceso][1][1]);
  }
  return 0;
}
