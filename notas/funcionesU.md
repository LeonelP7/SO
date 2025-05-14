# Expresiones regulares:
`regcomp(&regex, patron, REG_EXTENDED | REG_NOSUB) != 0`
| Parte           | Significado                            |                                          |
| --------------- | -------------------------------------- | ---------------------------------------- |
| `regcomp`       | Compila una expresión regular POSIX    |                                          |
| `&regex`        | Donde se guarda la expresión compilada |                                          |
| `patron`        | Cadena con la expresión regular        |                                          |
| \`REG\_EXTENDED | REG\_NOSUB\`                           | Usa regex extendido, sin capturar grupos |
| `!= 0`          | Detecta si hubo un error al compilar   |

# Función `strcspn` en C

## 📘 Descripción

La función `strcspn` calcula la longitud del segmento inicial de una cadena (`str1`) que **no contiene ningún carácter** de otra cadena (`str2`).

---

## 🧠 Sintaxis

```c
size_t strcspn(const char *str1, const char *str2);
