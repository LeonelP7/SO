#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N_HILOS 3
#define EOL '\0'
#define MAX_BUFF 256

pthread_cond_t condPpal = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int turno = 99, nLineas = 0;
typedef struct {
  char *linea;
  int estado;  // 0 rechazado 1 aceptado
  float valor;
} Transaccion;

Transaccion *vect;

void *filtroUno(void *args);
void *filtroDos(void *args);
void *filtroTres(void *args);

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Faltan args :(\n");
  }
  pthread_t *pidHilos = calloc(N_HILOS, sizeof(pthread_t));
  FILE *f = fopen(argv[1], "r");
  fscanf(f, "%d", &nLineas);

  vect = calloc(nLineas, sizeof(Transaccion));

  pthread_create(&pidHilos[0], NULL, filtroUno, NULL);
  pthread_create(&pidHilos[1], NULL, filtroDos, NULL);
  pthread_create(&pidHilos[2], NULL, filtroTres, NULL);

  fgetc(f);

  printf("Lineas: \n");
  for (int i = 0; i < nLineas; i++) {
    vect[i].linea = calloc(MAX_BUFF, sizeof(char));
    fgets(vect[i].linea, MAX_BUFF, f);
    printf("%s", vect[i].linea);
  }
  fclose(f);

  turno = 0;
  pthread_cond_signal(&condPpal);

  pthread_mutex_lock(&mutex);
  while (turno < N_HILOS) {
    pthread_cond_wait(&cond3, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  printf("\nResultado: \n");
  for (int i = 0; i < nLineas; i++) {
    printf("%s [VALIDA: %d]\n", vect[i].linea, vect[i].estado);
  }

  for (int i = 0; i < N_HILOS; i++) {
    pthread_join(pidHilos[i], NULL);
  }
  free(vect);
  free(pidHilos);
  return 0;
}

void *filtroUno(void *args) {

  pthread_mutex_lock(&mutex);
  while (turno != 0) {
    pthread_cond_wait(&condPpal, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  regex_t regex;
  const char *patron =
      "^TRX: [a-zA-Z0-9]+ -> [a-zA-Z0-9]+ : ([0-9]+(\\.[0-9]+)?)$";
  regmatch_t matches[2];
  char numStr[50];
  float num = 0;

  if (regcomp(&regex, patron, REG_EXTENDED) != 0) {
    fprintf(stderr, "No se pudo compilar la expresion regular\n");
    exit(EXIT_FAILURE);
  }

  printf("soy el hilo 1:\n");
  for (int i = 0; i < nLineas; i++) {
    printf("%s", vect[i].linea);
    vect[i].linea[strcspn(vect[i].linea, "\n")] = 0;
    if (regexec(&regex, vect[i].linea, 2, matches, 0) == 0) {
      strncpy(numStr, &vect[i].linea[matches[1].rm_so],
              matches[1].rm_eo - matches[1].rm_so);
      numStr[matches[1].rm_eo - matches[1].rm_so] = '\0';

      num = strtof(numStr, NULL);
      vect[i].valor = num;
      vect[i].estado = 1;
      printf("%s", vect[i].linea);
    } else {
      vect[i].estado = 0;
    }
  }
  pthread_mutex_lock(&mutex);
  turno++;
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond1);
  pthread_exit(0);
}

void *filtroDos(void *args) {
  pthread_mutex_lock(&mutex);
  while (turno != 1) {
    pthread_cond_wait(&cond1, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  for (int i = 0; i < nLineas; i++) {
    if (!(vect[i].estado == 1 && vect[i].valor > 0 && vect[i].valor <= 10000)) {
      vect[i].estado = 0;
    }
  }

  pthread_mutex_lock(&mutex);
  turno++;
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond2);
  pthread_exit(0);
}

void *filtroTres(void *args) {
  pthread_mutex_lock(&mutex);
  while (turno != 2) {
    pthread_cond_wait(&cond2, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  for (int i = 0; i < nLineas; i++) {
    if (!(strstr(vect[i].linea, "hacker") == NULL &&
          strstr(vect[i].linea, "cashout") == NULL &&
          strstr(vect[i].linea, "vault") == NULL && vect[i].estado == 1)) {
      vect[i].estado = 0;
    }
  }

  pthread_mutex_lock(&mutex);
  turno++;
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond3);
  pthread_exit(0);
}