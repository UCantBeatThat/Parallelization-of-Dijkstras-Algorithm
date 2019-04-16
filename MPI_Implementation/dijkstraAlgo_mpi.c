#include<stdio.h>
#include <mpi.h>
#include<stdlib.h>
#include<stdbool.h>
#include <limits.h>
#define MAX_SIZE 20 //max number of nodes

int G[MAX_SIZE][MAX_SIZE];  //adjacency matrix
bool visited[MAX_SIZE]; //nodes done
int D[MAX_SIZE]; //distance
int path[MAX_SIZE]; //we came to this node from
int N; //actual number of nodes
int size, rank;
MPI_Status status;
int flag;
int all_paths[MAX_SIZE];

int readAdjacencyMatrix(){
    printf("Enter number of nodes: ");
    scanf("%d", &N);

    printf("Enter cost matrix (%d*%d elements, enter 99999 if the node is not reachable):\n", N, N);
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
          scanf("%d", &G[i][j]);
        }
    }

    return N;
}

void dijkstraAlgorithm(int s){
    int i,j;
    int tmp, x;
    int pair[2];
    int tmp_pair[2];
    
    //initialize all the nodes
    for(i=rank;i<N;i+=size){
        D[i]=G[s][i];
        visited[i]=false;
        path[i]=s;
    }
    
    //set src as visited node
    visited[s]=true;
    path[s]=-1;
    
    //compute the distance of the shortest path from the source node
    for(j=1;j<N;j++){
        x=99999;
        tmp=99999;
        for(i=rank;i<N;i+=size){
            if(!visited[i] && D[i]<tmp){ //find the neighbour node with least distance
                x=i;
                tmp=D[i];
            }
        }   
        //store that neighbour node and its corresponding distance in an array
        pair[0]=x;
        pair[1]=tmp;
        
        //compute the global minimum of distances obtained from all the processes
        if(rank!=0){
            MPI_Send(pair,2,MPI_INT,0,rank,MPI_COMM_WORLD);
        }
        else{
            for(i=1;i<size;++i){
                MPI_Recv(tmp_pair,2,MPI_INT,i,i,MPI_COMM_WORLD, &status);
                if(tmp_pair[1]<pair[1]){
                    pair[0]=tmp_pair[0];
                    pair[1]=tmp_pair[1];
                }
            }
        }
        
        //broadcast the obtained least distance node and distance
        MPI_Bcast(pair,2,MPI_INT,0,MPI_COMM_WORLD);
        x=pair[0];
        D[x]=pair[1];
        visited[x]=true; //mark the node as visited
        
        //check if any other node can be visited efficiently through the obtained node
        for(i=rank;i<N;i+=size){
            if(!visited[i] && D[i]>D[x]+G[x][i]){
                D[i]=D[x]+G[x][i];
                path[i]=x;
            }
        }
    }

    //to obtain the paths from all the processes
    MPI_Reduce(path, all_paths, N, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
}


int main(int argc, char** argv){

    double t1, t2;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int src_node, dest_node;
    if(rank==0){
        //read the adjacency matrix along with the actual number of nodes
        N = readAdjacencyMatrix();
        printf("\nEnter Source Node(0-%d): ", N-1);
        scanf("%d", &src_node);
        printf("\nEnter Destination Node(0-%d): ", N-1);
        scanf("%d", &dest_node);
        t1=MPI_Wtime();
    }

    //broadcast required data to all the processes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(G, MAX_SIZE*MAX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&src_node, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //call the algorithm with the choosen node
    dijkstraAlgorithm(src_node);

    if(rank==0){
        t2=MPI_Wtime();
        //check the results with some output from G[][] and D[]
        printf("\n--------------------------------------\nNode = %d \tDistance = %d \n", dest_node, D[dest_node]);
        int temp;
        temp = dest_node;
        printf("\nPATH: ");
        int req_path[N];
        int count = 0;
        while(all_paths[temp]!=-1){
        	req_path[count++] = all_paths[temp];
            //printf(" <-- %d ", req_path[count-1]);
            temp = all_paths[temp];
        }

        for(int i=count-1;i>=0;i--){
        	printf(" %d -->", req_path[i]);
        }
        printf(" %d ", dest_node);

        FILE *fin = fopen("output.txt", "a");
        printf("\n--------------------------------------\n\nTIME ELAPSED: %lf ms\n\n",(t2-t1)*1000);
        fprintf(fin, "%d\t| %lf\n",size, (t2-t1)*1000);
    }

    MPI_Finalize();
}


/*
Sample-Input:
9
0 4 99999 99999 99999 99999 99999 8 99999 
4 0 8 99999 99999 99999 99999 11 99999
99999 8 0 7 99999 4 99999 99999 2
99999 99999 7 0 9 14 99999 99999 99999
99999 99999 99999 9 0 10 99999 99999 99999
99999 99999 4 14 10 0 2 99999 99999 
99999 99999 99999 99999 99999 2 0 1 6
8 11 99999 99999 99999 99999 1 0 7
99999 99999 2 99999 99999 99999 6 7 0
*/