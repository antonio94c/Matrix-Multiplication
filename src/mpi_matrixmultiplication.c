/******************************************************************************
* FILE: mpi_matrixmultiplication.c
* DESCRIPTION:  
*   MPI Matrix Multiply - C Version
*   In this code, the master task distributes a matrix multiply
*   operation to numtasks worker tasks.
* AUTHOR: Antonio Calabria, Università degli studi di Salerno.
* LAST REVISED: 13/06/17
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define SIZE 3200         
#define MASTER 0               
#define FROM_MASTER 1          
#define FROM_WORKER 2          

int main (int argc, char *argv[])
{

double  start,finish;
int	numtasks, taskid,numslave, source, dest, mtype, rows, n_row, extra, offset;       
int     **a,**b; 
int     i,j,k,count;        

MPI_Status status;
MPI_Init(&argc,&argv);
start = MPI_Wtime();
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);



/*Dinamic Allocation Matrix A e B*/

a =(int**) malloc(SIZE * sizeof(int*));
for(i=0; i < SIZE; i++)
    a[i] =(int*) malloc(SIZE * sizeof(int*));

b =(int**) malloc(SIZE * sizeof(int*));
for(i=0; i < SIZE; i++)
    b[i] =(int*) malloc(SIZE * sizeof(int*));
 
/*Initializing Matrix A e B*/ 
srand(0);
for(i=0; i<SIZE; i++)
      for(j=0; j<SIZE; j++)
            b[i][j]= rand()%10;
for(i=0; i<SIZE; i++)
      for(j=0; j<SIZE; j++)
            a[i][j]= rand()%10;

/**************************** master task ************************************/
   if (taskid == MASTER)
   {
      printf("matrix_multiplication has started with %d tasks.\n",numtasks);

      /*Stampa Delle Matrci*
      printf("***************************MatrixA**********************************\n");
      for(i=0; i<SIZE; i++){
         for(j=0; j<SIZE; j++)
            printf("%d,",a[i][j]);
         printf("\n");
      }
      printf("***************************MatrixB**********************************\n");
      for(i=0; i<SIZE; i++){
         for(j=0; j<SIZE; j++)
            printf("%d,",b[i][j]);
         printf("\n");
      } */
      

      n_row = SIZE/numtasks;
      extra = SIZE%numtasks;
      offset = 0;
      mtype = FROM_MASTER;
      rows = 0;
      numslave = numtasks-1; 
     
      offset = offset + n_row; 
      
      /* Invio degli indici agli slave */
      for(dest=1; dest<=numslave; dest++)
      {
         rows = (dest <= extra) ? n_row+1 : n_row;   	
         printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         offset = offset + rows;
      }

      int **c;

      /*Allocazione Dinamica Matrice C */
      c =(int**) malloc(SIZE * sizeof(int*));
      for(i=0; i < SIZE; i++)
          c[i] =(int*) malloc(SIZE * sizeof(int*));

      /*Inizializzazione Matrice C*/
      for(i=0; i<SIZE; i++)
         for(j=0; j<SIZE; j++)
             c[i][j]= 0; 
      
      /*Calcolo del risultato*/
      for(i=0; i < n_row; i++)
         for(j=0; j < SIZE; j++)
            for(k=0; k < SIZE; k++)
                c[i][j] = c[i][j] + (a[i][k]*b[k][j]);

     

      
      /* Ricevo i risultati dagli slave */
      mtype = FROM_WORKER;
 
      for(i=1; i<=numslave; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         printf("Receiving %d rows to task %d offset=%d\n",rows,i,offset);
         int *ris; 
         ris = malloc(rows*SIZE*sizeof(int));
         
         MPI_Recv(ris, rows*SIZE, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
                        
      
         count=0;
         for(k=offset; k < rows+offset; k++){
            for(j=0; j < SIZE; j++){
                c[k][j] = ris[count];
                count++;
            }
         }
        
         free(ris);
         
      }

      /* Stampa del risultato * 
      printf("******************************************************\n");
      printf("Result Matrix:\n");
      for(i=0; i<SIZE; i++)
      {
         printf("\n"); 
         for(j=0; j<SIZE; j++) 
            printf("%d ", c[i][j]);
      }
      printf("\n******************************************************\n");
      printf("Done.\n"); */     

      
      free(*c);      
      finish=MPI_Wtime();
      printf("End of Computing process %d, in time %.2fs\n",taskid, finish-start);
   }
     

/**************************** slave task ************************************/
   if(taskid > MASTER)
   {
      /*Ricevo indici dal master*/
      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);

      int *ris;
      ris = malloc(rows*SIZE*sizeof(int));

      /*Inizializzazione vettore ris*/
      for(i=0; i<SIZE*rows; i++)
         ris[i]=0; 

      /*Calcolo del risultato*/
      count=0;
      for(i=offset; i < rows+offset; i++){
         for(j=0; j < SIZE; j++){
            for(k=0; k < SIZE; k++)
                ris[count] += (a[i][k]*b[k][j]);
            count++;
          }
      }      

      /*Invio del risultato al master*/
      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(ris, rows*SIZE, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);

      free(ris);
   }

free(*a);
free(*b);


MPI_Finalize();
return 0;
}
