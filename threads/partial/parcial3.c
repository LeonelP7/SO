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
pthread_cond_t cond4 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int turno = 99;
int nGrupos = 0;
int cheq = 0, etiq = 0, sumaV = 0, consol = 0;
int lCorruptas = 0, pid50000 = 0, nocturnas = 0;
int nLineas = 0;
int nFiles = 0;
void *chequeo(void *args);
void *etiquetado(void *args);
void *suma(void *args);
void *consolidacion(void *args);

typedef struct {
  char *linea;
  int estado;
  int pid;
  //int cpu;
  int segundos;
  int milisegundos;
  int flag_big;
  int flag_night;
} Traza;
Traza *vect = NULL;

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
  nGrupos = nFiles/4;
  if (nGrupos<1){nGrupos = 1;}
  printf("nGrupos: %d\n", nGrupos);
  pidHilos = (pthread_t *)calloc(nGrupos * 4, sizeof(pthread_t));

  int k = 0;
  for (int i = 0; i < nGrupos; i++) {
    int *args = (int *)calloc(1, sizeof(int));
    int *args2 = (int *)calloc(1, sizeof(int));
    int *args3 = (int *)calloc(1, sizeof(int));
    int *args4 = (int *)calloc(1, sizeof(int));
    *args = i;
    *args2 = i;
    *args3 = i;
    pthread_create(&pidHilos[k++], NULL, chequeo, args);
    pthread_create(&pidHilos[k++], NULL, etiquetado, args2);
    pthread_create(&pidHilos[k++], NULL, suma, args3); 
    pthread_create(&pidHilos[k++], NULL, consolidacion, NULL); 
  }

  for (int i = 0; i < nFiles; i++) {
    f = fopen(files[i], "r");
    fscanf(f, "%d", &nLineas);
    fgetc(f);

    free(vect);
    vect = (Traza *)calloc(nLineas, sizeof(Traza));

    printf("Archivo: %s\n", files[i]);
    for (int i = 0; i < nLineas; i++) {
      vect[i].linea = (char *)calloc(MAX_BUFF, sizeof(char));
      fgets(vect[i].linea, MAX_BUFF, f);
      vect[i].linea[strcspn(vect[i].linea, "\n")] = EOL;
      printf("vect[%d].linea:%s\n", i, vect[i].linea);
    }
    fclose(f);

    turno = 0;
    pthread_cond_broadcast(&condPpal);

    pthread_mutex_lock(&mutex);
    while (turno < 4) {
      pthread_cond_wait(&cond4, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    printf("ARCHIVO: %s\n", files[i]);
    printf("LINEAS CORRUPTAS: %d\n", lCorruptas);
    printf("PID > 50000: %d\n", pid50000);
    printf("TOTAL EVENTOS MADRUGADA: %d\n", nocturnas);
    pthread_mutex_lock(&mutex);
    turno = 0;
    cheq = 0;
    etiq = 0;
    sumaV = 0;
    consol = 0;
    lCorruptas = 0;
    pid50000 = 0;
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
  pthread_cond_destroy(&cond4);
  return 0;
}

void *chequeo(void *args) {
  int index = *((int *)args);
  printf("index: %d",index);
  free(args);
  for (int k = 0; k < nFiles; k++) {
    pthread_mutex_lock(&mutex);
    while (turno != 0) {
      pthread_cond_wait(&condPpal, &mutex);
    }
    pthread_mutex_unlock(&mutex);


    int delta = (int)ceil(nLineas / nGrupos);
    //int delta = 2;
    int ini = delta * index;
    int fin = ((delta + ini) > nLineas) ? nLineas : delta + ini;
    printf("ini: %d fin: %d\n",ini,fin);

    regex_t regex;
    const char *patron = "^[0-9]+;([0-9]+);([0-9]{1,5}\\.)([0-9]{1,3})$";
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
        vect[i].pid = num;

        strncpy(numStr, &vect[i].linea[matches[2].rm_so],
                matches[2].rm_eo - matches[2].rm_so);
        numStr[matches[2].rm_eo - matches[2].rm_so] = '\0';
        num = strtof(numStr, NULL);
        vect[i].segundos = num;

        strncpy(numStr, &vect[i].linea[matches[3].rm_so],
                matches[3].rm_eo - matches[3].rm_so);
        numStr[matches[3].rm_eo - matches[3].rm_so] = '\0';
        num = strtof(numStr, NULL);
        vect[i].milisegundos = num;

        if ((vect[i].pid >= 1) &&
            (vect[i].segundos >= 0 && vect[i].segundos <= 99999) &&
            (vect[i].milisegundos >= 0 && vect[i].milisegundos <= 999)) {
          vect[i].estado = 1;
          printf("%d %d:%d %d\n", vect[i].pid, vect[i].segundos,vect[i].milisegundos,vect[i].estado);
        } else {
          vect[i].estado = 0;
          pthread_mutex_lock(&mutex);
          lCorruptas++;
          pthread_mutex_unlock(&mutex);
        }
      } else {
        vect[i].estado = 0;
        pthread_mutex_lock(&mutex);
        lCorruptas++;
        pthread_mutex_unlock(&mutex);
      }
    }
    pthread_mutex_lock(&mutex);
    cheq++;
    if (cheq == nGrupos) {
      turno++;
      printf("turno: %d\n",turno);
      pthread_cond_broadcast(&cond1);
    }
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

void *etiquetado(void *args) {
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
      if (vect[i].estado && vect[i].pid > 50000) {
        vect[i].flag_big = 1;
      }
      if (vect[i].estado && (vect[i].segundos >= 00 && vect[i].segundos <= 50)) {
        vect[i].flag_night = 1;
      }
    }
    pthread_mutex_lock(&mutex);
    etiq++;
    if (etiq == nGrupos) {
      turno++;
      printf("turno: %d\n",turno);
      pthread_cond_broadcast(&cond2);
    }
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

void *suma(void *args) {
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
      if (vect[i].flag_big && vect[i].estado) {
        pid50000++;
      }
      if (vect[i].flag_night && vect[i].estado) {
        nocturnas++;
      }
    }
    pthread_mutex_lock(&mutex);
    sumaV++;
    if (sumaV == nGrupos) {
      turno++;
      printf("turno: %d\n",turno);
      pthread_cond_broadcast(&cond3);
    }
    pthread_mutex_unlock(&mutex);
  }

  pthread_exit(0);
}

void *consolidacion(void *args){
  for (int k = 0; k < nFiles; k++) {
    //printf("k de cons %d\n",k);
    pthread_mutex_lock(&mutex);
    while (turno != 3) {
      pthread_cond_wait(&cond3, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&mutex);
    //printf("soy cons estoy molestando\n");
    consol++;
    //printf("consol %d\n",consol);
    if(consol==nGrupos)turno++;
    printf("turno: %d\n",turno);
    pthread_cond_broadcast(&cond4);
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

