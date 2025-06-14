
# Funciones útiles para trabajar con strings en C

En el lenguaje de programación C, las cadenas de caracteres (*strings*) se manejan como arreglos de caracteres terminados en nulo 0. La biblioteca estándar `<string.h>` proporciona una serie de funciones útiles para manipularlas.

A continuación, se describen algunas de las funciones más comunes y útiles:

---

## `strlen`

- **Descripción:** Devuelve la longitud de una cadena, sin contar el carácter nulo 0.
- **Firma:** `size_t strlen(const char *str);`
- **Ejemplo:**
  ```c
  printf("%zu", strlen("hola")); // Imprime 4
  ```

---

## `strcmp`

- **Descripción:** Compara dos cadenas carácter por carácter.
- **Firma:** `int strcmp(const char *s1, const char *s2);`
- **Retorna:**
  - 0 si son iguales,
  - <0 si `s1` es menor que `s2`,
  - >0 si `s1` es mayor que `s2`.
- **Ejemplo:**
  ```c
  if (strcmp("abc", "abd") < 0) {
      printf("abc es menor");
  }
  ```

---

## `strncmp`

- **Descripción:** Compara hasta `n` caracteres de dos cadenas.
- **Firma:** `int strncmp(const char *s1, const char *s2, size_t n);`
- **Ejemplo:**
  ```c
  strncmp("auto", "autor", 4); // retorna 0
  ```

---

## `strcpy`

- **Descripción:** Copia una cadena en otra.
- **Firma:** `char *strcpy(char *dest, const char *src);`
- **Nota:** Asegúrate de que `dest` tenga suficiente espacio.
- **Ejemplo:**
  ```c
  char buffer[20];
  strcpy(buffer, "hola");
  ```

---

## `strncpy`

- **Descripción:** Copia hasta `n` caracteres.
- **Firma:** `char *strncpy(char *dest, const char *src, size_t n);`
- **Ejemplo:**
  ```c
  strncpy(buffer, "hola", 3); // copia "hol"
  ```

---

## `strcat`

- **Descripción:** Concatena dos cadenas.
- **Firma:** `char *strcat(char *dest, const char *src);`
- **Ejemplo:**
  ```c
  char buffer[20] = "Hola ";
  strcat(buffer, "mundo");
  ```

---

## `strstr`

- **Descripción:** Busca una subcadena dentro de otra.
- **Firma:** `char *strstr(const char *haystack, const char *needle);`
- **Retorna:** Un puntero a la primera aparición de `needle`.
- **Ejemplo:**
  ```c
  char *ptr = strstr("Hola mundo", "mundo"); // ptr apunta a "mundo"
  ```

---

## `strchr`

- **Descripción:** Busca la primera aparición de un carácter.
- **Firma:** `char *strchr(const char *str, int c);`
- **Ejemplo:**
  ```c
  char *p = strchr("banana", 'n'); // apunta a la primera 'n'
  ```

---

## `strrchr`

- **Descripción:** Busca la última aparición de un carácter.
- **Firma:** `char *strrchr(const char *str, int c);`
- **Ejemplo:**
  ```c
  char *p = strrchr("banana", 'n'); // apunta a la última 'n'
  ```

---

## `strcspn`

- **Descripción:** Calcula la longitud del segmento inicial que no contiene ningún carácter del segundo argumento.
- **Firma:** `size_t strcspn(const char *str1, const char *str2);`
- **Ejemplo:**
  ```c
  size_t pos = strcspn("abc123", "123"); // retorna 3 (posición donde empieza un dígito)
  ```

---

## `strtok`

- **Descripción:** Divide una cadena en tokens (fragmentos) usando delimitadores.
- **Firma:** `char *strtok(char *str, const char *delim);`
- **Ejemplo:**
  ```c
  char str[] = "uno,dos,tres";
  char *token = strtok(str, ",");
  while (token != NULL) {
      printf("%s\n", token);
      token = strtok(NULL, ",");
  }
  ```

---

## Recomendaciones

- Siempre incluir `#include <string.h>`.
- Maneja cuidadosamente los tamaños de los buffers para evitar desbordamientos.
- Evita modificar cadenas literales directamente.

---

> 📝 Estas funciones son fundamentales para tareas comunes de análisis, comparación, búsqueda y modificación de cadenas en C.
