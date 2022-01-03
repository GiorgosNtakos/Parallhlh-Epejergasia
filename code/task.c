#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>

int start, end, threads, elements, id;
double gettime(void) {
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return tv.tv_sec + 1e-6 * tv.tv_usec;
}
typedef struct {
    unsigned int first;
    unsigned int second;
} arc;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadFile(int fvert[], int svert[])
{
    FILE *in_file;
        int i=0,j=0;

    in_file = fopen("dag.txt", "r");
    
    ////////////////////////////////////////

    if (in_file == NULL)
    {
        printf("Can't open file for reading.\n");
    }
    else
    {
    	fscanf(in_file, "%*d");
    	fscanf(in_file, "%*d");
    	fscanf(in_file, "%*d");
    	while(!feof(in_file))
    	{ 
		fscanf(in_file, "%d", &fvert[i]);
       	fscanf(in_file, "%d", &svert[i]);
       	i++;
	}

    }
    fclose(in_file);
}

////////////////////////////////////////////////////////////

void SetSizes(int *size, int *order)
{
    FILE *in_file;

    in_file = fopen("dag.txt", "r");
    
    
    fscanf(in_file, "%d", *&order);
    fscanf(in_file, "%*d");
    fscanf(in_file, "%d", *&size);
    
	printf("%d %d\n", *size, *order);
 
    *size=*size-1;
    
    
    
    fclose(in_file);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Find out if a vertex has no incoming arcs */
static unsigned int is_root(const arc *graph, const int *arcs, unsigned int size,unsigned int v)
{
    unsigned int a, root = 1;
    for (a = 0; a < size && root; a++) {
        root = !arcs[a] || graph[a].second != v;
    }
    return root;
}

/* Get the vertices with no incoming arcs */
static unsigned int get_roots(const arc *graph, const int *arcs, unsigned int size,
        unsigned int order, unsigned int *vertices)
{
    unsigned int v, vertices_size = 0;
    for (v = 0; v < order; v++) {
        if (is_root(graph, arcs, size, v)) {
            vertices[v] = 1;
            vertices_size++;
        }
    }
    return vertices_size;
}

unsigned int topological_sort(const arc *graph, unsigned int size, unsigned int order,unsigned int sorted[])
{
    int *vertices =(int *) malloc(order * sizeof(int));
    int *arcs =(int *) malloc(size * sizeof(int));

    int v, a, vertices_size, sorted_size = 0,arcs_size = size;
    if (!(vertices && arcs && sorted)) {
        free(vertices);
        free(arcs);
        free(sorted);
        sorted = NULL;
        return 0;
    }
        
        
    /* All arcs start off in the graph */
    for (a = 0; a < size; a++) {
        arcs[a] = 1;
    }
    /* Get the vertices with no incoming edges */
        vertices_size = get_roots(graph, arcs, size, order, vertices);
        
	#pragma omp parallel  num_threads(4) 
   	{
	while(vertices_size>0)
	{
   	id = omp_get_thread_num();
        threads = omp_get_num_threads();
  	
	    /* Main loop */
	    for(v=0; v<order; v++)
	    {
	        /* Get first vertex */
#pragma omp task
		if(vertices[v] == 1)
	        {
	        /* Remove from vertex set */
	        vertices[v] = 0;

	        vertices_size--;
	        /* Add it to the sorted array */
	        sorted[sorted_size] = v;

	        sorted_size++;
	        /* Remove all arcs connecting it to its neighbours */
		#pragma omp taskwait
	        for (a = 0; a < size; a++) {

	            if (arcs[a] && graph[a].first == v) {
	                arcs[a] = 0;
	              
	                arcs_size--;
	               
	                /* Check if neighbour is now a root */
	                
	                if (is_root(graph, arcs, size, graph[a].second)) {
	                    vertices[graph[a].second] = 1;

	                    vertices_size++;

	                }
	                
	            }
	         }
	        }
	    }
	   }
	}
	
	
    free(vertices);
    free(arcs);
    return arcs_size == 0;
}
/* Connect two arcs */
void arc_connect(arc *arcs, unsigned int first, unsigned int second, unsigned int *pos)
{
    arcs[*pos].first = first;
    arcs[*pos].second = second;
    (*pos)++;
}


int main(void)
{	

	double c0,c1;
	
	
    int size; /* Arcs */
    int order; /* Vertices */
    unsigned int i = 0,j=0;
    unsigned int *sorted=(int *) malloc(5000* sizeof(int));
    unsigned int acyclic;

    SetSizes(&size, &order);
    int fvert[size],svert[size];
    ReadFile(fvert, svert);
    arc *arcs =(arc *) malloc(size * sizeof(arc));

    printf("%d %d\n", size, order);
    
    for (j=0;j<size;j++)
    {
    	arc_connect(arcs, fvert[j], svert[j], &i);
    }
    
    c0=gettime();
#pragma omp parallel num_threads(4)
{
#pragma omp single nowait
{
    acyclic = topological_sort(arcs, size, order, sorted);
    printf("Graph is acyclic: %u\n", acyclic);
    if (!acyclic)
    {printf("The Diagram has at least one Cycle!!!\n");}
    else
	{	
	for (i = 0; i < order; i++) {
        printf("%u ", sorted[i]);
}
}
    }
}
	
c1=gettime();
    putchar('\n');

    free(sorted);
    free(arcs);
	printf("O xronos ektelesis einai %lf seconds \n",c1-c0);

    return 0;
}
