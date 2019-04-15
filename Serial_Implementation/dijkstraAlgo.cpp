#include<iostream>
#include<climits>
using namespace std;
int main(){
	/*
	4 4
	1 2 6
	2 3 2
	1 3 2
	1 0 2
	*/
	int n,e;
	cin>>n>>e;
	int** edges=new int*[n];
	for(int i=0;i<n;i++){
		edges[i]=new int[n];
	}
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			edges[i][j]=0;
		}
	}
	for(int i=0;i<e;i++){
		int f,s,w;
		cin>>f>>s>>w;
		edges[f][s]=w;
		edges[s][f]=w;
	}
	bool* visited=new bool[n];
	int* distance=new int[n];
	for(int i=0;i<n;i++){
		visited[i]=false;
	}
	for(int i=0;i<n;i++){
		distance[i]=INT_MAX;
	}
	//cosidering 0 as the source vertex
	distance[1]=0;
	//traverse to all the vertices
	for(int i=0;i<n-1;i++){
		//we should select the vertex with the min distance and that should be unvisited as well
		int best_distance=INT_MAX;		
		int vertex_to_work=-1;
		for(int vertex=0;vertex<n;vertex++){
			if((distance[vertex]<best_distance)&&(!visited[vertex])){
				best_distance=distance[vertex];
				vertex_to_work=vertex;
			}
		}
		//mark this as visited
		visited[vertex_to_work]=true;
		//explore all the unvisited neighbours
		for(int i=0;i<n;i++){
			if(i==vertex_to_work){continue;}
			if(edges[vertex_to_work][i]&&!visited[i]){
				//calculate current distance
				int cur_distance=distance[vertex_to_work]+edges[vertex_to_work][i];
				//if i have a better deal then
				if(cur_distance<distance[i]){
					distance[i]=cur_distance;
				} 
			}
		}
	}
	for(int i=0;i<n;i++){
		cout<<i<<" "<<distance[i]<<endl;
	}
}
