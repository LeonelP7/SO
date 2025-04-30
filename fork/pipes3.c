#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    int fd[2];
    int fe[2];
    int arr[]= {1,3,5,7,9,2,4,6,8,10};
    int arrSize = sizeof(arr)/sizeof(arr[0]);
    int start, end;

    if(pipe(fd) == -1 || pipe(fe) == -1){
        return 1;
    }

    int id1, id2;
    id1 = fork();
    if(id1 == -1){
        return 2;
    }
    if (id1 != 0)
    {
        id2 = fork();
        if(id2 == -1){
            return 3;
        }
    }
    
    if(id1 == 0){ //primer hijo
        start = 0;
        end = arrSize/3;
    }else{
        if(id2 == 0){ //segundo hijo
            start = arrSize/3;
            end = 2*(arrSize/3);
        }else{ //padre
            start = 2*(arrSize/3);
            end = arrSize;
        }
    }

    int sum = 0;
    for(int i = start; i < end; i++){
        sum += arr[i];
    }
    printf("Sum: %d\n",sum);

    if(id1 == 0){ //primer hijo
        close(fd[0]);
        if(write(fd[1],&sum,sizeof(int)) == -1){
            return 4;
        }
        close(fd[1]);
    }else{
        if(id2 == 0){ //segundo hijo
            close(fe[0]);
            if(write(fe[1],&sum,sizeof(int)) == -1){
                return 5;
            }
            close(fe[1]);
        }else{ //padre
            int sumChildren1, sumChildren2;
            while(wait(NULL) >0){
                printf("Waiting...\n");
            }
            close(fd[1]);
            close(fe[1]);
            if(read(fd[0],&sumChildren1,sizeof(int)) == -1 || read(fe[0],&sumChildren2,sizeof(int)) == -1){
                return 6;
            }
            close(fd[0]);
            close(fe[0]);
            printf("Total sum: %d\n",sum+sumChildren1+sumChildren2);
        }
    }

    return 0;
}
