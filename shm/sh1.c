#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

void handler(int s) {}

int Kill(pid_t pid, int sig)
{
    usleep(10000);
    return kill(pid, sig);
}

void create_index(void **m, int rows, int cols, size_t sizeElement)
{
    int i;
    size_t sizeRow = cols * sizeElement;
    m[0] = m + rows;
    for (i = 1; i < rows; i++)
    {
        m[i] = (m[i - 1] + sizeRow);
    }
}

unsigned int sizeof_dm(int rows, int cols, size_t sizeElement)
{
    size_t size;
    size = rows * sizeof(void *);        // indexSize
    size += (cols * rows * sizeElement); // Data size
    return size;
}

int main(int argc, char const *argv[])
{
    signal(SIGUSR1, handler);
    pid_t hijo;
    double **m = NULL;
    int rows = 3, cols = 3;
    
    size_t sizeMatrix = sizeof_dm(rows,cols,sizeof(double));
    int shmId = shmget(IPC_PRIVATE,sizeMatrix,IPC_CREAT|S_IRUSR | S_IWUSR);
    create_index((void*)m,rows,cols,sizeof(double));

    if (!(hijo = fork()))
    {
        pause();
        m = shmat(shmId,NULL,0);
        //printf("hola soy el hijo \n");
        for (int i = 0; i < rows; i++)
        {
           //printf("Ando Escribiendo en pantalla");
            for (int j = 0; j < cols; j++)
            {
                printf("[%f]", m[i][j]);
            }
            printf("\n");
        }
        shmdt(m);
        shmctl(shmId,IPC_RMID,NULL);
    }
    else
    {
        //printf("hola soy el padre \n");
        m = shmat(shmId,NULL, 0);
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                //printf("Ando Escribiendo en la matriz\n");
                m[i][j] = (i*cols) + j;
            }
        }
        shmdt(m);  
        Kill(hijo, SIGUSR1);
        wait(NULL);
    }

    return EXIT_SUCCESS;
}
