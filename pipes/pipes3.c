#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define EOL '\0'
#define MAX_READ 512

int main(int argc, char const *argv[]) {
  pid_t padre = getpid();
  int nProceso = 0;
  int fd1[3][2][2];

  char buff[MAX_READ];
  int nRead;

  // int fd2[2][2];

  for (int i = 0; i < 2; i++) {
    pipe(fd1[i][0]);
    pipe(fd1[i][1]);
  }

  for (nProceso = 0; nProceso < 2; nProceso++) {
    if (!fork()) {
      pipe(fd1[2][0]);
      pipe(fd1[2][1]);
      if (!fork()) {
        nProceso = 3;
        break;
      }
      break;
    }
  }

  if (nProceso == 2) {
    do
    {
        printf("Write 'fin' to exit.\n");
        fgets(buff,MAX_READ,stdin);
        if (strlen(buff) > 1)
        {
            buff[strlen(buff) - 1] = EOL; 
        }
        
        

    } while (condition);
    
  }


  return 0;
}
