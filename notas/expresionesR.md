# Guía para Trabajar con Expresiones Regulares en C

## 📘 ¿Qué son las expresiones regulares?

Las expresiones regulares (regex) son secuencias de caracteres que definen un patrón de búsqueda. En C, puedes usar expresiones regulares para buscar, modificar o validar cadenas de texto basadas en patrones específicos.

En C, las expresiones regulares se gestionan con la biblioteca `regex.h`, que proporciona una interfaz para compilar y ejecutar patrones regex.

---

## 🧠 Conceptos Básicos

### Expresión Regular

Una **expresión regular** es un patrón que describe un conjunto de cadenas. Puedes usar expresiones regulares para hacer coincidir patrones, buscar texto o realizar reemplazos en una cadena.

**Ejemplo:**  
La expresión regular `^abc` coincidiría con cualquier cadena que comience con "abc".

### Funciones Principales

Las principales funciones que proporciona la biblioteca `regex.h` en C son:

- `regcomp()`: Compila una expresión regular.
- `regexec()`: Ejecuta la expresión regular sobre una cadena.
- `regfree()`: Libera la memoria asociada con la expresión regular compilada.

### Sintaxis de las Expresiones Regulares

Algunas de las construcciones más comunes para las expresiones regulares en C son:

- `.`: Coincide con cualquier carácter excepto los saltos de línea.
- `*`: Coincide con 0 o más repeticiones del elemento anterior.
- `+`: Coincide con 1 o más repeticiones del elemento anterior.
- `?`: Hace que el elemento anterior sea opcional (0 o 1 vez).
- `^`: Coincide con el inicio de la cadena.
- `$`: Coincide con el final de la cadena.
- `[]`: Define un conjunto de caracteres. Ejemplo: `[a-z]` coincide con cualquier letra minúscula.
- `()`: Agrupa patrones para aplicar operadores como `*`, `+`, etc.
- `|`: Operador OR, coincide con uno de los patrones especificados.

---

## 🛠️ Cómo Trabajar con Expresiones Regulares en C

### 1. **Incluir la Biblioteca Regex**

Para trabajar con expresiones regulares en C, debes incluir la biblioteca `regex.h`:

```c
#include <regex.h>
```

### 2. **Compilar la Expresión Regular**

Antes de usar una expresión regular, debes compilarla con la función `regcomp()`:

```c
regex_t regex;
const char *patron = "tu_patron_aqui";

if (regcomp(&regex, patron, REG_EXTENDED | REG_NOSUB) != 0) {
    fprintf(stderr, "No se pudo compilar la expresión regular\n");
    return 1;
}
```

### 3. Ejecutar la Expresión Regular

Una vez compilada la expresión regular, puedes usar `regexec()` para buscar coincidencias:

```c
char cadena[] = "texto_a_validar";
if (regexec(&regex, cadena, 0, NULL, 0) == 0) {
    printf("Coincide con el patrón\n");
} else {
    printf("No coincide con el patrón\n");
}
```


# Uso de `regexec` y `regmatch_t` en C

## ¿Qué es `regexec`?

`regexec` es una función de la biblioteca `<regex.h>` en C que permite comparar una cadena de texto con una expresión regular previamente compilada usando `regcomp`.

```c
int regexec(const regex_t *preg, const char *string,
            size_t nmatch, regmatch_t pmatch[], int eflags);
```

### Parámetros:

- `preg`: Puntero a la expresión regular compilada (`regex_t`).
- `string`: Cadena de entrada que se desea validar.
- `nmatch`: Número de grupos a capturar (incluido el grupo 0, que representa toda la coincidencia).
- `pmatch`: Arreglo de estructuras `regmatch_t` donde se almacenan las posiciones de coincidencias.
- `eflags`: Banderas adicionales (generalmente `0`).

---

## ¿Qué es `regmatch_t`?

`regmatch_t` es una estructura que representa el índice de inicio y fin de una coincidencia dentro del texto analizado.

```c
typedef struct {
    regoff_t rm_so;  // start offset (índice inicial)
    regoff_t rm_eo;  // end offset (índice final)
} regmatch_t;
```

- `rm_so`: Índice del primer carácter de la coincidencia.
- `rm_eo`: Índice del carácter siguiente al último de la coincidencia.

---

## ¿Cómo capturar subgrupos?

Los **subgrupos** en una expresión regular se definen entre paréntesis `()`.

Ejemplo de expresión con un subgrupo para capturar un número decimal:

```c
const char *patron = "^TRX: [a-zA-Z0-9_]+ -> [a-zA-Z0-9_]+ : ([0-9]+(\.[0-9]+)?)$";
```

Esta expresión tiene dos subgrupos:

1. `([0-9]+(\.[0-9]+)?)`: todo el número decimal (subgrupo 1).
2. `(\.[0-9]+)?`: parte decimal opcional (subgrupo 2).

---

## Ejemplo de uso

```c
regex_t regex;
regmatch_t matches[3]; // Incluye grupo 0 y 2 subgrupos
char linea[] = "TRX: A1 -> B2 : 25.75";

regcomp(&regex, patron, REG_EXTENDED);
if (regexec(&regex, linea, 3, matches, 0) == 0) {
    // matches[1] contiene los índices del número capturado
    char numero_str[50];
    int inicio = matches[1].rm_so;
    int fin = matches[1].rm_eo;
    strncpy(numero_str, &linea[inicio], fin - inicio);
    numero_str[fin - inicio] = '\0'; // Null-terminar la cadena
    float valor = strtof(numero_str, NULL);
    printf("Valor extraído: %.2f\n", valor);
}
regfree(&regex);
```

---

## Notas

- Siempre verificar si `regexec` devuelve `0`, lo que indica una coincidencia.
- El grupo `matches[0]` siempre representa **toda la coincidencia** completa.
- Los grupos `matches[1]`, `matches[2]`, etc., representan **subgrupos entre paréntesis**.
- Es importante terminar manualmente la subcadena extraída con `'
