#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main (int argc, char* argv[]){
    int id1 = fork();
    int id2 = fork();

    if(id1 == 0){
        if(id2 == 0){
            printf("Im the process y\n");
        }else{
            printf("Im the process x\n");
        }

    }else{
        if(id2 == 0){
            printf("Im the process z\n");
        }else{
            printf("Im the parent process\n");
        }
    }

    while(wait(NULL) != -1 || errno != ECHILD){
        printf("Waited for a child to finish.\n");
    }
}


