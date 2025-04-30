#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    /* code */
    int id = fork();
    if(id == 0){
        sleep(1);
    }

    printf("Current ID: %d, Parent ID: %d\n", getpid(), getppid());
    int result = wait(NULL);
    if(result == -1){
        printf("No children to wait for\n");

    }else{
        printf("%d finished\n", result);
    }
    return 0;
}
