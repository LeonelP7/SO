#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    /* code */

    int fd[2];
    // fd[0] read
    // fd[1] write

    if(pipe(fd) == -1){
        printf("An error ocurred opening the pipe.");
        return 1;
    }

    int id = fork();
    if (id == -1)
    {
        printf("An error ocurred with fork.");
        return 2;
    }
    if (id == 0)
    {
        close(fd[0]);
        int x;
        printf("Hey im the son. Insert an int: ");
        scanf("%d",&x);
        if(write(fd[1], &x, sizeof(int)) == -1){
            printf("An error ocurred writing.");
            return 3;
        }
        close(fd[1]);
    }else{
        close(fd[1]);
        int y;
        if(read(fd[0],&y,sizeof(int)) == -1){
            printf("An error ocurred reading.");
            return 4;
        }
        printf("Hey im the father. The number is: %d\n",2*y);
        close(fd[0]);
    }
    
     

    return 0;
}
