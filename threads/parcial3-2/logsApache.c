#include <math.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define EOL '\0'
#define MAX_BUFF 256

pthread_cond_t condPpal = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int turno = 99;
int nGrupos = 0;
int vali = 0, fil = 0, conte = 0;
int malFormadas = 0, cod400 = 0, nocturnas = 0;
int nLineas = 0;
int nFiles = 0;
void *validacion(void *args);
void *filtrado(void *args);
void *conteo(void *args);

typedef struct {
  char *linea;
  int estado;
  int cod;
  int hora;
  int minutos;
  int cod400;
  int nocturna;
} Log;
Log *vect = NULL;

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Faltan args :(\n");
  }

  pthread_t *pidHilos = NULL;
  FILE *f = fopen(argv[1], "r");

  char **files = NULL;
  fscanf(f, "%d", &nFiles);
  files = (char **)calloc(nFiles, sizeof(char *));
  fgetc(f);
  for (int i = 0; i < nFiles; i++) {
    files[i] = (char *)calloc(MAX_BUFF, sizeof(char));
    fgets(files[i], MAX_BUFF, f);
    files[i][strcspn(files[i], "\n")] = EOL;
  }
  fclose(f);

  printf("nFiles: %d\n", nFiles);
  nGrupos = nFiles / 3;
  printf("nGrupos: %d\n", nGrupos);
  pidHilos = (pthread_t *)calloc(nGrupos * 3, sizeof(pthread_t));

  int k = 0;
  for (int i = 0; i < nFiles / 3; i++) {
    int *args = (int *)calloc(1, sizeof(int));
    int *args2 = (int *)calloc(1, sizeof(int));
    int *args3 = (int *)calloc(1, sizeof(int));
    *args = i;
    *args2 = i;
    *args3 = i;
    pthread_create(&pidHilos[k++], NULL, validacion, args);
    pthread_create(&pidHilos[k++], NULL, filtrado, args2);
    pthread_create(&pidHilos[k++], NULL, conteo, args3);
  }

  for (int i = 0; i < nFiles; i++) {
    f = fopen(files[i], "r");
    fscanf(f, "%d", &nLineas);
    fgetc(f);

    free(vect);
    vect = (Log *)calloc(nLineas, sizeof(Log));

    //printf("Archivo: %s\n", files[i]);
    for (int i = 0; i < nLineas; i++) {
      vect[i].linea = (char *)calloc(MAX_BUFF, sizeof(char));
      fgets(vect[i].linea, MAX_BUFF, f);
      vect[i].linea[strcspn(vect[i].linea, "\n")] = EOL;
      // printf("vect[%d].linea:%s\n", i, vect[i].linea);
    }
    fclose(f);

    turno = 0;
    pthread_cond_broadcast(&condPpal);

    pthread_mutex_lock(&mutex);
    while (turno < 3) {
      pthread_cond_wait(&cond3, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    printf("ARCHIVO: %s\n", files[i]);
    printf("MAL FORMADAS: %d\n", malFormadas);
    printf("COD 400-402: %d\n", cod400);
    printf("INCIDENTES NOCTURNOS: %d\n", nocturnas);
    pthread_mutex_lock(&mutex);
    turno = 0;
    vali = 0;
    fil = 0;
    conte = 0;
    malFormadas = 0;
    cod400 = 0;
    nocturnas = 0;
    pthread_mutex_unlock(&mutex);
  }

  for (int i = 0; i < k; i++) {
    pthread_join(pidHilos[i], NULL);
  }
  free(pidHilos);
  free(files);
  free(vect);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condPpal);
  pthread_cond_destroy(&cond1);
  pthread_cond_destroy(&cond2);
  pthread_cond_destroy(&cond3);
  return 0;
}

void *validacion(void *args) {
  int index = *((int *)args);
  free(args);
  for (int k = 0; k < nFiles; k++) {
    pthread_mutex_lock(&mutex);
    while (turno != 0) {
      pthread_cond_wait(&condPpal, &mutex);
    }
    pthread_mutex_unlock(&mutex);


    int delta = (int)ceil(nLineas / nGrupos);
    int ini = delta * index;
    int fin = ((delta + ini) > nLineas) ? nLineas : delta + ini;

    regex_t regex;
    const char *patron =
        "^([0-9]+);([0-9]+):([0-9]+);([0-9]{1,3}\\.){3}[0-9]{1,3}$";
    regmatch_t matches[4];
    char numStr[50];
    float num = 0;

    if (regcomp(&regex, patron, REG_EXTENDED) != 0) {
      fprintf(stderr, "No se pudo compilar la expresion regular\n");
      exit(EXIT_FAILURE);
    }


    for (int i = ini; i < fin; i++) {
      if (regexec(&regex, vect[i].linea, 4, matches, 0) == 0) {
        strncpy(numStr, &vect[i].linea[matches[1].rm_so],
                matches[1].rm_eo - matches[1].rm_so);
        numStr[matches[1].rm_eo - matches[1].rm_so] = '\0';

        num = strtof(numStr, NULL);
        vect[i].cod = num;

        strncpy(numStr, &vect[i].linea[matches[2].rm_so],
                matches[2].rm_eo - matches[2].rm_so);
        numStr[matches[2].rm_eo - matches[2].rm_so] = '\0';
        num = strtof(numStr, NULL);
        vect[i].hora = num;

        strncpy(numStr, &vect[i].linea[matches[3].rm_so],
                matches[3].rm_eo - matches[3].rm_so);
        numStr[matches[3].rm_eo - matches[3].rm_so] = '\0';
        num = strtof(numStr, NULL);
        vect[i].minutos = num;
        if ((vect[i].cod >= 100 && vect[i].cod <= 599) &&
            (vect[i].hora >= 00 && vect[i].hora <= 23) &&
            (vect[i].minutos >= 00 && vect[i].minutos <= 59)) {
          vect[i].estado = 1;
          // printf("%d %d:%d %d\n", vect[i].cod, vect[i].hora,
          // vect[i].minutos,vect[i].estado);
        } else {
          vect[i].estado = 0;
          pthread_mutex_lock(&mutex);
          malFormadas++;
          pthread_mutex_unlock(&mutex);
        }
      } else {
        vect[i].estado = 0;
        pthread_mutex_lock(&mutex);
        malFormadas++;
        pthread_mutex_unlock(&mutex);
      }
    }
    pthread_mutex_lock(&mutex);
    vali++;
    if (vali == nGrupos) {
      turno++;
      pthread_cond_broadcast(&cond1);
    }
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

void *filtrado(void *args) {
  int index = *((int *)args);
  free(args);
  for (int k = 0; k < nFiles; k++) {

    pthread_mutex_lock(&mutex);
    while (turno != 1) {
      pthread_cond_wait(&cond1, &mutex);
    }
    pthread_mutex_unlock(&mutex);


    int delta = (int)ceil(nLineas / nGrupos);
    int ini = delta * index;
    int fin = ((delta + ini) > nLineas) ? nLineas : delta + ini;


    for (int i = ini; i < fin; i++) {
      if (vect[i].estado && (vect[i].cod >= 400 && vect[i].cod <= 402)) {
        vect[i].cod400 = 1;
      }
      if (vect[i].estado && (vect[i].hora >= 00 && vect[i].hora <= 5) &&
          (vect[i].minutos >= 00 && vect[i].minutos <= 59)) {
        vect[i].nocturna = 1;
      }
    }
    pthread_mutex_lock(&mutex);
    fil++;
    if (fil == nGrupos) {
      turno++;
      pthread_cond_broadcast(&cond2);
    }
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

void *conteo(void *args) {
  int index = *((int *)args);
  free(args);
  for (int k = 0; k < nFiles; k++) {

    int delta = (int)ceil(nLineas / nGrupos);
    int ini = delta * index;
    int fin = ((delta + ini) > nLineas) ? nLineas : delta + ini;

    pthread_mutex_lock(&mutex);
    while (turno != 2) {
      pthread_cond_wait(&cond2, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    for (int i = ini; i < fin; i++) {
      if (vect[i].cod400 && vect[i].estado) {
        cod400++;
      }
      if (vect[i].nocturna && vect[i].estado) {
        nocturnas++;
      }
    }
    pthread_mutex_lock(&mutex);
    conte++;
    if (conte == nGrupos) {
      turno++;
      pthread_cond_broadcast(&cond3);
    }
    pthread_mutex_unlock(&mutex);
  }

  pthread_exit(0);
}