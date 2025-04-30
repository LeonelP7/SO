#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handler(int s) {}

int Kill(pid_t pid, int sig)
{
    usleep(10000);
    return kill(pid, sig);
}

int main(int argc, char const *argv[])
{
    /* code */

    signal(SIGUSR1, handler);
    int idx = 0, count;
    pid_t *vect, *vect2;
    pid_t padre;
    if (argc < 2)
    {
        /* code */
        printf("faltan args");
        exit(1);
    }
    count = atoi(argv[1]);
    vect = calloc(count, sizeof(pid_t));
    vect2 = calloc(2, sizeof(pid_t));
    padre = getpid();
    for (idx = 0; idx < count; idx++)
    {
        /* code */
        vect[idx] = fork();
        if (!vect[idx])
        {
            for (int i = 0; i < 2; i++)
            {
                /* code */
                vect2[i] = fork();
                if (vect2[i] == 0)
                {
                    break;
                }
            }
            break;
        }
    }

    int iteraciones = 2;

    for (int i = 0; i < iteraciones; i++)
    {
        /* code */
        if (idx == count)
        {
            /* code */
            printf("padre: %d\n", getpid());
            Kill(vect[idx - 1], SIGUSR1);
            pause();
            printf("padre\n");
        }
        else
        { // todos los hijos
            /* code */
            pause();
            pid_t p;
            pid_t p2;
            pid_t p3;
            if (padre == getppid())
            {
                printf("soy: %d\n", getpid());
                /* code */
                for (int i = 1; i >= 0; i--)
                {
                    /* code */
                    p2 = vect2[i];
                    Kill(p2, SIGUSR1);
                    pause();

                    // printf("llamando a: %d miPadre: %d idx: %d\n", p2, getppid(), idx);
                    printf("soy: %d\n", getpid());
                }
                p = idx == 0 ? getppid() : vect[idx - 1];
                Kill(p, SIGUSR1);
            }
            else
            {
                p3 = getppid();
                printf("llamando a: %d miPadre: %d idx: %d\n", p3, getppid(), idx);
                Kill(p3, SIGUSR1);
            }
        }
    }
    return 0;
}