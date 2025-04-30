#include <stdio.h>
#include <unistd.h>

int main() {
    /*
    int x = fork();
    if (x) {
        fork();
    } else {
        fork();
        fork();
    }
    printf("Proceso\n");
    return 0;
    */

    /*
    fork();
    fork();
    if (fork()) {
        fork();
    }
    fork();
    printf("Proceso\n");
    */

    /*
    int a = fork();
    if (a == 0) {
        fork();
        fork();
    } else {
        fork();
    }
    printf("Proceso\n");
    */

    return 0;

    
}
