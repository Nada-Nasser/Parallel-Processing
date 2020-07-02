#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>  
#include <unistd.h>


#define MAX_PROCESSES 50 // 50 = number of files
#define nAllFiles 50
#define root 0

int FileNumber[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,
                    11,12,13,14,15,16,17,18,19,20,
                    21,22,23,24,25,26,27,28,29,30,
                    31,32,33,34,35,36,37,38,39,40,
                    41,42,43,44,45,46,47,48,49,50};

int getResult(int *myFiles ,char *myResult ,int nFiles , char* query);// search in the Files to fill mtResult array and return size of it.
int isQueryAnswer(char* line ,char query[2000]);
char* FileName(int b);

int main(int argc , char * argv[])
{
    int my_rank , nProcesses , masterID = 0 , participants ,nFilesPerSlave , recvCnt , nResults , QueryLen , ResultLen;
	char Result[50000];
	int sumAllResults; // number of results from all slaves.
	int AllResultsLen[50000]; // n_Result at each slave
	
    int displs[MAX_PROCESSES];
    int send_counts[MAX_PROCESSES];
    
	int myFiles[MAX_PROCESSES]; // files that slave #(my_rank) search in.
    char myResult[50000]; // array that contains results at each slave.
    char query[200];
	
	FILE *ResultFile;
	
	struct timeval start, end;
    long mtime, secs, usecs;
	
    int i , FilesCounter,startCnt , TempSum = 0; // counters
    int tag = 0;		/* tag for messages	*/

    MPI_Status status;	/* return status for 	*/
    MPI_Init( &argc, &argv ); // initialize MPI

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // return the id (rank) of the process
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses); // return number of processes

//-----------------------------------------------------------------------------------------
    if ( nProcesses > MAX_PROCESSES ) // to make sure that #slaves not greater than number of files.
		participants = MAX_PROCESSES; // to make sure that each slave will search in atleast one file.
    else
		participants = nProcesses;
//----------------------------------------------------------------------------------------
    if(my_rank < participants)
    {
		// ceil(a/b) = (a / b) + ((a % b) != 0);
        //nFilesPerSlave = (nAllFiles/participants) + ((nAllFiles%participants) != 0);// 50 / participants
		nFilesPerSlave = nAllFiles/participants;
		recvCnt = nFilesPerSlave; // all slaves will search in (nFilesPerSlave) files, except the last slave.

        if(my_rank == participants-1) // number of file for the last slave.
            recvCnt = nAllFiles - (nFilesPerSlave*(participants-1));

        if(my_rank == root)
        {
			TempSum = 0;
			FilesCounter = 50;
            // set a  sendCount and startIndex for each participants
            do{
				i = 0 ;
				for ( i = 0 ; i < participants && FilesCounter > 0  ; i++)
				{
					if(FilesCounter - nFilesPerSlave < 0)
					{
						send_counts[i] += FilesCounter;
						TempSum+= FilesCounter;
						FilesCounter -=  FilesCounter;
					}
					else
					{
						send_counts[i] += nFilesPerSlave;
						TempSum+= nFilesPerSlave;
						FilesCounter -=  nFilesPerSlave;
					}							
				}
			}while(FilesCounter > 0);
			
			printf("\n%i FIles ",TempSum);
			
			FilesCounter = 0;
			for(i = 0 ; i < participants ; i++)
				FilesCounter+= send_counts[i];
			
			printf("\n%i sum " , FilesCounter);
			
			startCnt = 0;
			for(i = 0 ; i < participants ; i++)
			{
				displs[i] = startCnt;
				startCnt+= send_counts[i];
			}
			
            printf("Enter your query: ");
            gets(query); // use gets instead of scanf to read spaces.
			QueryLen = strlen(query)+1;	

        }
		
		MPI_Bcast(&QueryLen , 1  , MPI_INT , 0 , MPI_COMM_WORLD);

        MPI_Bcast(query , QueryLen , MPI_CHAR , 0 , MPI_COMM_WORLD);
		
		//MPI_Scatterv(&FileNumber, send_counts, displs, MPI_INT, &myFiles , recvCnt, MPI_INT, root, MPI_COMM_WORLD);
		MPI_Scatter(send_counts , 1 , MPI_INT , &recvCnt , 1 , MPI_INT , root , MPI_COMM_WORLD);
		if(my_rank == root)
		{
			for(i = 0 ; i < send_counts[root] ; i++)
				myFiles[i] = FileNumber[i];
			
			for(i = 1 ; i < participants ; i++)
				MPI_Send(FileNumber+displs[i] , send_counts[i] , MPI_INT , i , tag , MPI_COMM_WORLD);
		}
		else
		{
			MPI_Recv(myFiles , recvCnt , MPI_INT , root , tag , MPI_COMM_WORLD,&status);
		}
		
		if(my_rank == root)
		{
			gettimeofday(&start, NULL);
		}

		nResults = 0;
	
		nResults = getResult(myFiles , myResult , recvCnt , query);
		
		ResultLen = strlen(myResult);
	
		
		MPI_Reduce(&nResults , &sumAllResults , 1 , MPI_INT , MPI_SUM , root , MPI_COMM_WORLD); // now root has sum of all results.
		
		MPI_Gather(&ResultLen , 1 , MPI_INT , AllResultsLen , 1 , MPI_INT , root , MPI_COMM_WORLD);
		
		if(my_rank != root)
		{
			MPI_Send(myResult , ResultLen , MPI_CHAR , root,tag ,MPI_COMM_WORLD);
		}
		else
		{

			ResultFile = fopen ("Result.txt", "w");
			
			fprintf(ResultFile,"Number of all results : %i\r\n" , sumAllResults);
			
			fprintf(ResultFile,"\r\nAll Results :\r\n");
			if(AllResultsLen[root] > 0)
				fprintf(ResultFile,"%s\n" , myResult);
			
			for(i = 1 ; i < participants ; i++)
			{
				MPI_Recv(Result , AllResultsLen[i] , MPI_CHAR ,i,tag, MPI_COMM_WORLD,&status);
				if(AllResultsLen[i] > 0)
					fprintf(ResultFile,"%s\n" ,Result);
			}
			
			fclose(ResultFile);
			
			gettimeofday(&end, NULL);
			secs  = end.tv_sec  - start.tv_sec;
			usecs = end.tv_usec - start.tv_usec;
			mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;
			printf("time: %ld millisecs\n", mtime);

		}
		
	}

    MPI_Finalize();
    return 0;
}

char* FileName(int b)
{
    char *result;
    char folder[] = "Aristo-Mini-Corpus/";
    char s1[] = "Aristo-Mini-Corpus P-";
    char s2[20]; // number
    char s3[] = ".txt";

    sprintf(s2, "%d", b); // convert int to string

    result = malloc(strlen(s1) + strlen(s2) + strlen(s3) + strlen(folder) + 1); // +1 for the null-terminator

    // in real code you would check for errors in malloc here
    strcpy(result,folder);
    strcat(result, s1);
    strcat(result, s2);
    strcat(result, s3);

    return result;
}


int getResult(int *myFiles ,char myResult[50000] ,int nFiles , char query[2000])// search in the Files to fill mtResult array and return size of it.
{
    int i = 0 , j = 0;
	//myResult = malloc(2000);

	int ResultLen = 0 , ResultCnt = 0;
    for(i = 0 ; i < nFiles ; i++)
    {
        char* filename = FileName(myFiles[i]);

        FILE * file;
        char * line = NULL;
        size_t len = 0;
        size_t lineLen;

        file = fopen(filename, "r");
			
	//	printf("%s\n" , filename);
		
        while ((lineLen = getline(&line, &len, file)) != -1) 
		{
			if(isQueryAnswer(line , query) == 1)
			{
				
				//ResultLen+=lineLen;
				ResultCnt++;
				if(j==0)
				{
					if (line)
					strcpy(myResult,line);
				}
				else
				{
					if (line)
					strcat(myResult,line);
				}
				j++;
				//break;
			}	
		}

        fclose(file);
        if (line)
            free(line);
    }
	
	return ResultCnt;
}

int isQueryAnswer(char* line ,char* Q)
{
	char* query = (char*) malloc(strlen(Q)+1);
	strcpy(query , Q);
	
	const char dilm[2] = " ";
    char *word;
   
   /* get the first word in the query */
   word = strtok(query, dilm);
  
   while( word != NULL )
   {  
		// Check if this word at the Line
		// if No
		// return 0.
		if(strstr(line, word) == NULL) {
			return 0;
		}
		word = strtok(NULL, dilm);
   }

   
   return 1;
}


/*
Match = true;
For each word in the query:
IF word not in CurrentSentence:
MatchScore = false;
IF MatchingScore is true:
Store Sentence;
ResultsFound += 1;
*/









