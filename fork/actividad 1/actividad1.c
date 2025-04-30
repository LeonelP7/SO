#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h> 

void error(char *msg) {
	perror(msg);
	exit(-1);
}

int leerNumeros(char *filename, int **vec)
{
	int c, numero, totalNumeros;
	FILE *infile;
	infile = fopen(filename, "r");
	if (!infile)
	{
		error("Error fopen\n");
	}
	fscanf(infile, "%d", &totalNumeros);
	*vec = (int *)calloc(totalNumeros, sizeof(int));
	if (!*vec)
	{
		error("error calloc");
	}
	for (c = 0; c < totalNumeros; c++)
	{
		fscanf(infile, "%d\n", &numero);
		(*vec)[c] = numero;
		//printf("%d\n", numero);
	}
	fclose(infile);
	return c;
}

void suma(int ini, int fin, int *vec)
{
	FILE *fp;
	fp = fopen("out.txt", "a");
	if (!fp)
	{
		error("Error fopen");
	}
	int sum = 0;
	for (int j = ini; j < fin; j++)
	{
		/* code */
		sum += vec[j];
	}
	printf("El hijo %d suma: %d\n", getpid(), sum);
	fprintf(fp, "%d\n", sum);
	fclose(fp);
}

int leerTotal(int nHijos)
{
	FILE *infile;
	int sumap1 = 0, total = 0;
	infile = fopen("out.txt", "r");
	if (!infile)
		error("Error padre archivo resultados");
	
	for (int i = 0; i < nHijos; i++){
		fscanf(infile, "%d", &sumap1);
		total+= sumap1;
		//fclose(infile);
	}
	return total;
}

int main(int argc, char const *argv[])
{
	/* code */

	int nHijos;
	printf("Escribe el numero de hijos: \n");
	scanf("%d", &nHijos);

	int *vec;
	int c = leerNumeros("test1.in", &vec);

	int delta = c / nHijos;
	int ini, fin;
	int sum;

	//FILE *fp = NULL;
	//fp = fopen("out.txt", "w");

	for (int i = 0; i < nHijos; i++)
	{
		if (!fork())
		{
			/* code */
			ini = delta * i;
			fin = i == (nHijos-1) ? c : ini + delta;
			suma(ini, fin, vec);
			return 0;
		}
	}
	//fclose(fp);
	

	for(int i = 0; i<nHijos; i++){
	    wait(NULL);
	}

    int total = leerTotal(nHijos);

	printf("el resultado de la suma: %d",total);
	remove("out.txt");

	return 0;
}