#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void printMatrix(int * mat , int r , int c);
int main(int argc, char *argv[])
{
	int row1 , col1 , row2 , col2 , i ,j,num;
    int *matrix1, *matrix2 , *Result;
	
	printf("%s\n", "write number of rows in the first matrix");
	scanf("%i", &row1);

	printf("%s\n", "write number of columns in the first matrix");
	scanf("%i", &col1);

	printf("%s\n", "write number of rows in the second matrix");
	scanf("%i", &row2);

	printf("%s\n", "write number of columns in the second matrix");
	scanf("%i", &col2);
	
	if(row1!= row2 || col1 != col2)
	{
		printf("\nERROR, two matrices don't have same dimensions\n");
		return 0;
	}
	matrix1 = malloc (row1*col1 * sizeof(int));
	matrix2 = malloc (row2*col2 * sizeof(int));
	
	/* Some initializations */	
	printf("%s\n", "Enter the first matrix :");
	for (i = 0; i <  row1*col1; i++){
		scanf("%i", &num);
		*(matrix1 + i) = num;
	}
	
	/* Some initializations */	
	printf("%s\n", "Enter the second matrix :");
	for (i = 0; i <  row2*col2; i++){
		scanf("%i", &num);
		*(matrix2 + i) = num;
	}
		
	Result  = malloc (row2*col2 * sizeof(int));
    #pragma omp parallel shared(matrix1,matrix2,row2,col2,Result) private(i)
    {
        #pragma omp for schedule(static)
        for (i = 0; i <  row2*col2; i++)
		{
			*(Result + i) = *(matrix2 + i) + *(matrix1 + i);
		}
    }   /* end of parallel region */
	
	printf("\nResult = \n");
	printMatrix(Result,row1,col1);
	return 0;
}

void printMatrix(int * mat , int r , int c)
{
	int i , j;
	for(i = 0 ; i < r ; i++)
	{
		for(j = 0 ; j < c ; j++)
			printf("%i  " , *(mat + i*c + j));
		printf("\n");
	}
}
