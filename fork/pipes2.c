#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(int argc, char const *argv[])
{
    int fd[2]; // 0 read, 1 write
    int arr[] = {1,2,3,9,8,7,6};
    int arrSize = sizeof(arr)/sizeof(arr[0]);
    int start, end;

    if(pipe(fd) == -1){
        return 1;
    }

    int id = fork();
    if(id == -1){
        return 2;
    }

    if(id == 0){
        start = 0;
        end = arrSize/2;
    }else{
        start = arrSize/2;
        end = arrSize;
    }

    int sum;
    for(int i = start; i < end; i++){
        sum += arr[i];
    }
    printf("Sum: %d\n",sum);

    if(id == 0){
        close(fd[0]);
        write(fd[1],&sum,sizeof(int));
        close(fd[1]);
    }else{
        wait(NULL);
        int sumChildren;
        close(fd[1]);
        read(fd[0],&sumChildren,sizeof(int));
        close(fd[0]);
        printf("Total sum: %d\n",sum+sumChildren);
    }

    return 0;
}
