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
