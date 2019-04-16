%%cu
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <cuda.h>
#include <cuda_runtime.h>

#define V 8
#define E 11
#define MAX_WEIGHT 1000000
#define TRUE    1
#define FALSE   0

typedef int boolean;

typedef struct
{
	int u;
	int v;
} Edge;


typedef struct 
{
	int title;
	boolean visited;	
}Vertex;


__device__ __host__ int findEdge(Vertex u, Vertex v, Edge *edges, int *weights)
{

	int i;
	for(i = 0; i < E; i++)
	{
		if((edges[i].u == u.title && edges[i].v == v.title) || (edges[i].v == u.title && edges[i].u == v.title))
		{
			return weights[i];
		}
	}

	return MAX_WEIGHT;
}


__global__ void initVertices(Vertex *vertices, Edge *edges, int* weights, int* length, int* updateLength, Vertex root){
    
    int i = threadIdx.x;
    
    if(vertices[i].title != root.title)
		{
			length[(int)vertices[i].title] = findEdge(root, vertices[i], edges, weights);
			updateLength[vertices[i].title] = length[(int)vertices[i].title];		
		}
		else{
			vertices[i].visited = TRUE;
		}
}


__global__ void findVertex(Vertex *vertices, Edge *edges, int *weights, int *length, int *updateLength, int* path)
{

	int u = threadIdx.x;

	if(vertices[u].visited == FALSE)
	{

		vertices[u].visited = TRUE;

		int v;
		for(v = 0; v < V; v++)
		{	
			int weight = findEdge(vertices[u], vertices[v], edges, weights);

			if(weight < MAX_WEIGHT)
			{	
				
				if(updateLength[v] > length[u] + weight)
				{
			        path[v] = u;
					    updateLength[v] = length[u] + weight;
				}
			}
		}
	}
}


__global__ void updatePaths(Vertex *vertices, int *length, int *updateLength)
{
	int u = threadIdx.x;
	if(length[u] > updateLength[u])
	{
		length[u] = updateLength[u];
		vertices[u].visited = FALSE;
	}

	updateLength[u] = length[u];
}



void printShortestPath(int *array, int dest, int *req_path, int count)
{
	printf("Shortest Path to Vertex: %d is %d\nPATH: ", dest, array[dest]);
	for(int i=count-1;i>=0;i--)
    {
        printf("%d-->", req_path[i]);
    }
    printf("%d", dest);
}



int main(void)
{
    
	Vertex *vertices;	
	Edge *edges;

	
	int *weights;
  	int *path;
  	int *len, *updateLength;
	
	Vertex *d_V;
	Edge *d_E;
	int *d_W;
	int *d_L;
	int *d_C, *d_P;
	
	int sizeV = sizeof(Vertex) * V;
	int sizeE = sizeof(Edge) * E;
	int size = V * sizeof(int);
	
	float runningTime;
	cudaEvent_t timeStart, timeEnd;
	
	cudaEventCreate(&timeStart);
	cudaEventCreate(&timeEnd);

	vertices = (Vertex *)malloc(sizeV);
	edges = (Edge *)malloc(sizeE);
	weights = (int *)malloc(E* sizeof(int));
  	path = (int *)malloc(V*sizeof(int));
	len = (int *)malloc(size);
	updateLength = (int *)malloc(size);

	Edge ed[E] = {{0, 4}, {0, 6}, {0,2}, {4,6}, {4,7}, {0, 7}, {7, 3}, {3, 1}, {2,5}, {2, 1}, {5,3}};
	int w[E] = {10, 90, 30, 20, 20, 50, 10, 20, 10, 10, 10};
	
	cudaMalloc((void**)&d_V, sizeV);
	cudaMalloc((void**)&d_E, sizeE);
	cudaMalloc((void**)&d_W, E * sizeof(int));
	cudaMalloc((void**)&d_L, size);
	cudaMalloc((void**)&d_C, size);
  	cudaMalloc((void**)&d_P, size);

	Vertex root = {7, FALSE};
	root.visited = TRUE;
	len[root.title] = 0;
	updateLength[root.title] = 0;

	int dest = 0;

	int i = 0;
	for(i = 0; i < V; i++)
	{
		Vertex a = { i , FALSE};
		vertices[i] = a;
    	path[i] = root.title;
	}

	for(i = 0; i < E; i++)
	{
		edges[i] = ed[i];
		weights[i] = w[i];
	}

    
	cudaMemcpy(d_V, vertices, sizeV, cudaMemcpyHostToDevice);
	cudaMemcpy(d_E, edges, sizeE, cudaMemcpyHostToDevice);
	cudaMemcpy(d_W, weights, E * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(d_L, len, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_C, updateLength, size, cudaMemcpyHostToDevice);
  	cudaMemcpy(d_P, path, size, cudaMemcpyHostToDevice);
    
    
 	initVertices<<<1, V>>>(d_V, d_E, d_W, d_L, d_C, root);
	
  	cudaMemcpy(len, d_L, size, cudaMemcpyDeviceToHost);
  	cudaMemcpy(updateLength, d_C, size, cudaMemcpyDeviceToHost);

	cudaEventRecord(timeStart, 0);
	
	cudaMemcpy(d_L, len, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_C, updateLength, size, cudaMemcpyHostToDevice);
	
	for(i = 1; i < V; i++)
	{
		findVertex<<<1, V>>>(d_V, d_E, d_W, d_L, d_C, d_P);
		updatePaths<<<1,V>>>(d_V, d_L, d_C);
	}	
	
	
	cudaEventRecord(timeEnd, 0);
	cudaEventSynchronize(timeEnd);
	cudaEventElapsedTime(&runningTime, timeStart, timeEnd);

	cudaMemcpy(len, d_L, size, cudaMemcpyDeviceToHost);
  	cudaMemcpy(path, d_P, size, cudaMemcpyDeviceToHost);
    
    int req_path[V];
    int temp = dest;
    int count = 0;
    while(temp!=root.title)
    {
    	req_path[count++] = path[temp];
        temp = path[temp];
    }
    
    printShortestPath(len, dest, req_path, count);

	printf("Running Time: %f ms\n", runningTime);

	free(vertices);
	free(edges);
	free(weights);
	free(len);
	free(updateLength);
	cudaFree(d_V);
	cudaFree(d_E);
	cudaFree(d_W);
	cudaFree(d_L);
	cudaFree(d_C);
	cudaFree(d_P);
	cudaEventDestroy(timeStart);
	cudaEventDestroy(timeEnd);
}