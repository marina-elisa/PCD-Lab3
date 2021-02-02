#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define SRAND_VALUE 1985


int getNeighbors(int **grid, int posX, int posY, int N, int *upperLine, int *lowerLine, int first, int last){
    int startX, startY, x, y, count = 0;
        startX = posX == 0 ? N - 1 : posX - 1;
        startY = posY == 0 ? N - 1 : posY - 1;
        for(int i = 0; i <= 2; i++){
            x = (startX + i) >= N ? (startX + i - N) : startX + i;
            for(int j = 0; j <= 2; j++){
                y = (startY + j) >= N ? (startY + j - N) : startY + j;
                if(x < first)
                    count += upperLine[y];
                else if( x > (last - 1))
                    count += lowerLine[y];
                else
                    count += grid[x][y];
            }
        }
        count -= grid[posX][posY];
        return count;
}

int main(int argc, char** argv){

    int processID;
    int nProcesses;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);

    int N = 2048;
    int nGeracoes = 4;

    int localSize, remainder, first, last;
    localSize = N/nProcesses;
    first = processID*localSize;
    last = (processID + 1)*localSize;
    
    int count;
    int **grid, **newGrid;
    grid = malloc(N*sizeof(int*));
    newGrid = malloc(N*sizeof(int*));
    for(int i = 0; i < N; i++){
        grid[i] = malloc(N*sizeof(int));
        newGrid[i] = malloc(N*sizeof(int));
    }
    srand(SRAND_VALUE);
    for(int i =0; i < N; i++){
        for(int j = 0; j < N; j++){
            grid[i][j] = rand()%2;
        }
    }
    int *upperLine, *lowerLine, *firstLine, *lastLine;
    upperLine = malloc(N*sizeof(int));
    lowerLine = malloc(N*sizeof(int));
    firstLine = malloc(N*sizeof(int));
    lastLine = malloc(N*sizeof(int));
    MPI_Status status;
    
    for(int i = 0; i < nGeracoes; i++){
        for(int j = 0; j < N; j++){
            firstLine[j] = grid[first][j];
            lastLine[j] = grid[last-1][j];
        }
        if(nProcesses > 1){
            if(processID%2 == 0){
                MPI_Recv( lowerLine , N , MPI_INT , (processID + 1)%nProcesses , 10 , MPI_COMM_WORLD , &status);
                MPI_Recv( upperLine , N , MPI_INT , (processID + nProcesses -1)%nProcesses , 20 , MPI_COMM_WORLD , &status);
                MPI_Send( firstLine , N , MPI_INT , (processID + nProcesses -1)%nProcesses , 10 , MPI_COMM_WORLD);
                MPI_Send( lastLine , N , MPI_INT , (processID + 1)%nProcesses , 20 , MPI_COMM_WORLD);
            }else{
                MPI_Send( firstLine , N , MPI_INT , (processID + nProcesses -1)%nProcesses , 10 , MPI_COMM_WORLD);
                MPI_Send( lastLine , N , MPI_INT , (processID + 1)%nProcesses , 20 , MPI_COMM_WORLD);
                MPI_Recv( lowerLine , N , MPI_INT , (processID + 1)%nProcesses , 10 , MPI_COMM_WORLD , &status);
                MPI_Recv( upperLine , N , MPI_INT , (processID + nProcesses -1)%nProcesses , 20 , MPI_COMM_WORLD , &status);
            }
        }else{
            for(int j = 0; j < N; j++){
                lowerLine[j] = firstLine[j];
                upperLine[j] = lastLine[j];
            }
        }
        for(int j = first; j < last; j++){
            for(int k = 0; k < N; k++){
                count = getNeighbors(grid,j,k,N,upperLine,lowerLine,first,last);
                if(count < 2){
                    newGrid[j][k] = 0;
                }else if(count >= 2 && count < 4 && grid[j][k] == 1){
                    newGrid[j][k] = 1;
                }else if(count >= 4 && grid[j][k] == 1){
                    newGrid[j][k] = 0;
                }else if(count == 3 && grid[j][k] == 0){
                    newGrid[j][k] = 1;
                }
            }  
        }
        for(int j = first; j < last; j++){
            for(int k = 0; k < N; k++){
                grid[j][k] = newGrid[j][k];
            }
        }
        
        int localSum = 0, totalSum = 0;
        for(int j = first; j < last; j++){
            for(int k = 0; k < N; k++){
                localSum += grid[j][k];
            }
        }
        MPI_Reduce( &localSum , &totalSum , 1 , MPI_INT , MPI_SUM , 0 , MPI_COMM_WORLD);
        if(processID == 0){
            printf("%d\n",totalSum);
        }
    }    
        
    MPI_Finalize();
}