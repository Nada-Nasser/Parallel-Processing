#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"
#include <stdlib.h>

int main(int argc , char * argv[])
{
    int my_rank , nProcesses , masterID = 0;
    int i,j,k,l, num , id , rowN , colN , rowResult;
    int *matrix1 , *matrix2 , row1 , row2 , col1 , col2 , *out , *myOutput , nCalcProcess , startIndex , *slaveOutput;
    int aveCalcForProcess , allCalc , calcCounter;

    int tag = 0;		/* tag for messages	*/
    MPI_Status status;	/* return status for 	*/
    MPI_Init( &argc, &argv ); // initialize MPI

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // return the id (rank) of the process
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses); // return number of processes

   // my_rank = 0; ///*****************************
   // nProcesses = 4; ///*************************

    if( my_rank == 0) // master core
    {
        printf( "%s\n", "in master core..");
        printf("%s\n", "write number of rows in the first matrix");
        scanf("%i", &row1);

        printf("%s\n", "write number of columns in the first matrix");
        scanf("%i", &col1);

        printf("%s\n", "write number of rows in the second matrix");
        scanf("%i", &row2);

        printf("%s\n", "write number of columns in the second matrix");
        scanf("%i", &col2);

         if(col1 != row2) {
            printf("row != col.\n");
            exit(1);
         }

        matrix1 = (int *)malloc(row1 * col1 * sizeof(int));
        matrix2 = (int*) malloc(row2 * col2 * sizeof(int));
        out = (int*) malloc(row1 * col2 * sizeof(int));

        printf("%s\n", "Enter the first matrix :");
        for (i = 0; i <  row1; i++)
            for (j = 0; j < col1; j++)
            {
                scanf("%i", &num);
                *(matrix1 + i*col1 + j) = num;
            }

        printf("%s\n", "Enter the second matrix :");
        for (i = 0; i <  row2; i++)
            for (j = 0; j < col2; j++)
            {
                scanf("%i", &num);
                *(matrix2 + i*col2 + j) = num;
            }

      //  aveCalcForProcess = ceil((row1*col2)/(float)nProcesses);
        aveCalcForProcess = ((row1*col2)/(float)nProcesses) + ((row1*col2) % nProcesses != 0);
        //(a / b) + ((a % b) != 0)

        allCalc = row1*col2;
        calcCounter = allCalc;
       // printf("average = %i\n" , aveCalcForProcess);
        for(id = 1 ; id < nProcesses ; id++)
        {
            if(calcCounter - aveCalcForProcess < 0)
            {
                nCalcProcess = calcCounter;
            }
            else
            {
                nCalcProcess = aveCalcForProcess;
            }
            startIndex = id*aveCalcForProcess;

            calcCounter-= nCalcProcess;

            /// send startIndex  , row1 , row2 , col1, col2, nCalcProcess , matrix1 , matrix2 .

            MPI_Send( &startIndex, 1, MPI_INT, id, tag, MPI_COMM_WORLD); // startIndex
            MPI_Send( &row1, 1, MPI_INT, id, tag, MPI_COMM_WORLD); // row1
            MPI_Send( &row2, 1, MPI_INT, id, tag, MPI_COMM_WORLD); // row2
            MPI_Send( &col1, 1, MPI_INT, id, tag, MPI_COMM_WORLD); // col1
            MPI_Send( &col2, 1, MPI_INT, id, tag, MPI_COMM_WORLD); // col2
            MPI_Send( &nCalcProcess, 1, MPI_INT, id, tag, MPI_COMM_WORLD); // col2

            MPI_Send(matrix1, row1*col1 , MPI_INT, id , tag, MPI_COMM_WORLD);
            MPI_Send(matrix2, row2*col2 , MPI_INT, id , tag, MPI_COMM_WORLD);

           // printf("for id %i\n" , id);

           // printf("nCalcProcess = %i  , startIndex = %i\n" , nCalcProcess, startIndex);
        }

        startIndex = 0;
        nCalcProcess = aveCalcForProcess;

     //   myOutput = (int*) malloc(nCalcProcess * sizeof(int));
        i=0;j=0;k=0;
        while(i < nCalcProcess)
        {
            rowN = startIndex/col2;
            colN = startIndex%col2;
            rowResult = 0;
            for(j=0;j<col2;j++)
            {
                // matrix1[rowN][j]
                // matrix2[j][colN]
                rowResult += ( (*(matrix1 + rowN*col1 + j)) * (*(matrix2 + j*col2 + colN)) );
             //   myOutput[k] = rowResult;
              //  k++;
            }
            printf("%i " , rowResult);

            *(out+startIndex) = rowResult;

            i++;
            startIndex++;
        }
		printf("\n");

        // receive:  size of output array , output array , startIndex(= index in the output matrix) , nCalcProcess

		nCalcProcess = 0;
        for( id = 1; id < nProcesses ; id++)
		{
            MPI_Recv(&k            , 1, MPI_INT, id, tag, MPI_COMM_WORLD, &status );
            if(k==0)
				break;
			
			slaveOutput = (int*) malloc(k * sizeof(int));

			MPI_Recv(slaveOutput   , k, MPI_INT, id, tag, MPI_COMM_WORLD, &status );
            MPI_Recv(&startIndex   , 1, MPI_INT, id, tag, MPI_COMM_WORLD, &status );
            MPI_Recv(&nCalcProcess , 1, MPI_INT, id, tag, MPI_COMM_WORLD, &status );
				
			printf("master receive from rank : %i\n" , id);
			printf("slave array :\n");
			for(i = 0 ; i < k ; i++)
				printf("%i " , slaveOutput[i] );
			
            i=0;j=0;
            while( i < nCalcProcess)
            {
                *(out+startIndex) = slaveOutput[i];
                
				printf("we stored %i at index = %i\n" , slaveOutput[i] , startIndex);
                startIndex++;
				i++;
            }
			free(slaveOutput);

		}

        //print output
        i=0;j=0;
        for(i=0;i<row1;i++){
            for(j=0;j<col2;j++)
                printf("%i " , *(out + i*col2 + j));
            printf("\n");
        }
		
	
		free(matrix1);
        free(matrix2);
        free(out);
            
    }
    else
    {
		printf("rank %i start recv.\n" , my_rank);
        // receive : startIndex  , row1 , row2  , col1, col2 , nCalcProcess , matrix1 , matrix2 .

        MPI_Recv(&startIndex   , 1          , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );
		MPI_Recv(&row1         , 1          , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );
		MPI_Recv(&row2         , 1          , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );
		MPI_Recv(&col1         , 1          , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );
		MPI_Recv(&col2         , 1          , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );
		MPI_Recv(&nCalcProcess , 1          , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );

        matrix1 = (int *)malloc(row1 * col1 * sizeof(int));
        matrix2 = (int*) malloc(row2 * col2 * sizeof(int));

        MPI_Recv(matrix1       , row1*col1  , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );
        MPI_Recv(matrix2       , row2*col2  , MPI_INT, masterID, tag, MPI_COMM_WORLD, &status );

		printf("rank %i start calc %i numbers start = %i.\n" , my_rank , nCalcProcess , startIndex);
        // calculate the output
		l = startIndex;
  //      printf("%i ncalc = " , nCalcProcess);
		if(l < row1*col2)
        {
            myOutput = (int*) malloc(nCalcProcess * sizeof(int));
            i=0;j=0;k=0;
            while(i < nCalcProcess)
            {
                rowN = l/col2;
                colN = l%col2;
                rowResult = 0;
                for(j=0;j<col2;j++)
                {
                    // matrix1[rowN][j]
                    // matrix2[j][colN]
                    rowResult += ( (*(matrix1 + rowN*col1 + j)) * (*(matrix2 + j*col2 + colN)) );
                }
				
				myOutput[k] = rowResult;
                k++;
                
				printf("%i " , rowResult);
                i++;
                l++;
            }
			
			printf("Rank %i finished \n" , my_rank);

            // send array of results , its size(max = nCalcProcess) and the startIndex , nCalcProcess.
            MPI_Send( &k           , 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
            MPI_Send( myOutput     , k, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
            MPI_Send( &startIndex  , 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
            MPI_Send( &nCalcProcess, 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2

			printf("Rank %i sent the result \n" , my_rank);

        }
		else{
			k=0;
			myOutput = (int*) malloc(1 * sizeof(int));
			MPI_Send( &k           , 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
            MPI_Send( myOutput     , 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
            MPI_Send( &startIndex  , 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
            MPI_Send( &nCalcProcess, 1, MPI_INT, masterID, tag, MPI_COMM_WORLD); // col2
		}
	//	free(matrix1);
    //    free(matrix2);
     //   free(myOutput);
           // free(out);
          //  free(slaveOutput);
    }
	
	MPI_Finalize();

    return 0;
}


            