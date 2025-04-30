#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

//using wait(), the parent process waits for the child process to finish 

int main(int argc, char const *argv[])
{
    /* code */
    int id = fork();
    int n;

    if(id == 0){
        n = 1;
    }else {
        n = 6;
    }

    if(id != 0){
        wait();
    }

    int i;
    for(i = n; i < n + 5; i++){
        printf("%d ", i);
        fflush(stdout);
    }

    if(id != 0){
        printf("\n");
    }

    return 0;
}
