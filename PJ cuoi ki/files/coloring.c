#include<stdio.h>
#include<string.h>

#define MAX 100

typedef struct {
    int node;
    int color; // color: 1, 2, 3, ...
    char style[20]; //filled or unfilled
} node_t;

typedef struct{
    node_t node;
    int edge1;
    int edge2;
} graphType;


//Initialize
void initialize(graphType *graph, int num_nodes, int num_edges){
    int i;
    for(i = 0; i <= num_nodes; i++){
        graph[i].node.color = 0;           //No color yet
        strcpy(graph[i].node.style, "Unfilled");
    }
}

void input_graph(FILE *fptr, graphType *graph, int num_nodes, int num_edges){
    int i;
    for(i = 1; i <= num_edges; i++){
        fscanf(fptr, "%d %d", &graph[i].edge1, &graph[i].edge2);
    }
    
    // for(i = 0; i < num_edges; i++){
    //     printf("%d-%d\n", graph[i].edge1, graph[i].edge2);
    // }
}

/*---Count the degree of each node---*/
void graph_degree(graphType *graph, int *count, int n){
    int i;

    for(i = 1; i <= n; i++){
        count[graph[i].edge1]++;  //=> count[i] = node[i]'s degree
        count[graph[i].edge2]++;
    }
    // for(i = 1; i <= n; i++){
    //     printf("Bac cua node %d: %d\n", i, count[i]);
    // }
}

void put_number(graphType *graph, int *count, node_t *V, int num_nodes, int num_edges){
    int count_max = count[1];
    int i, k, j;
    //Find degree max
    for(i = 2; i <= num_edges; i++){
        if(count_max < count[i]){
            count_max = count[i];
        }
    }

    int n = num_nodes;
    //Choose a <= (k-1) node => Vn
    for(i = 1; i <= num_nodes; i++){
        if(count[i] <= (count_max -1)){
            V[n].node = i;
            break;
        }
    }

    int mark[MAX] = {0}; //Array for mark if we have met that case before -> avoid repetition
    int m = n;
    for(i = num_nodes; i >= 1; i--){
        //Find neighbour -> neighbour = {V[n-1], V[n-1], ...}
        for(k = 1; k <= num_edges; k++){
            if(graph[k].edge1 == V[i].node){ // -> edge2 is neighbour
                for(j = num_nodes; j >= 1; j--){
                        if(graph[k].edge2 == V[j].node){
                            mark[k] = 1;
                        }
                }
                if(mark[k] == 0){      // This case is the 1st time
                    
                    V[m-1].node = graph[k].edge2;
                    m--; 
                    mark[k] = 1;
                }
            }
            else if(graph[k].edge2 == V[i].node){
                for(j = num_nodes; j >= 1; j--){
                        if(graph[k].edge1 == V[j].node){
                            mark[k] = 1;
                        }
                }
                if(mark[k] == 0){
                    V[m-1].node = graph[k].edge1;
                    m--;
                    mark[k] = 1;
                }
            }
        }
    }

    // for(i = 1; i <= num_nodes; i++){
    //     printf("V[%d] = %d\n", i, V[i].node);
    // }
}

/*---Check if a is linked to b or not---*/
int is_link_to(node_t a, node_t b, graphType *graph, int num_edges){
    int i;
    int mark = 0;
    for(i = 0; i < num_edges; i++){
        if((graph[i].edge1 == a.node && graph[i].edge2 == b.node) || (graph[i].edge1 == b.node && graph[i].edge2 == a.node)){
            mark = 1;
        }
    }

    if(mark == 1){
        return 1; //Have link between a & b
    }
    else{
        return 0; //Don't have link between a & b
    }
}

void greedy_algorithm(graphType *graph, int *count, node_t *V, int num_nodes, int num_edges){
    int i, k, j;
    int mark = 0;  //Mark = 0 -> don't have link
                   //Mark = 1 -> have link
    int colorAvalaible[MAX] = {0};
    /*Initialize V[1]*/
    V[1].color = 1;
    strcpy(V[1].style, "Filled");

    for(i = 2; i <= num_nodes; i++){
        mark = 0;
        j = 1;
        //Reset array colorAvalaible after loops
        for(k = 0; k < num_nodes; k++){
            colorAvalaible[k] = 0;
        }


        for(k = 1; k < i; k++){
            if(is_link_to(V[k], V[i], graph, num_edges) == 1){   //V[k] is linked to V[i]
                mark = 1;
                colorAvalaible[V[k].color] = -1; //Cannot assigned this color anymore
                for(j = 1; j < num_nodes; j++){
                    if(colorAvalaible[j] != -1){
                        break;
                    }
                }
            }
        }
        if(mark == 1){
            V[i].color = j;
            strcpy(V[i].style, "Filled");
        }

        if(mark == 0){  //V[k] isn't linked to V[i]
            V[i].color = 1;
            strcpy(V[i].style, "Filled");
        }

    }
}

void print_result(graphType *graph, node_t *V, int num_nodes, int num_edges){
    int i;
    printf("---Danh sách canh---\n");
    for(i = 1; i <= num_edges; i++){
        printf("%d -- %d\n", graph[i].edge1, graph[i].edge2);
    }

    printf("\n---Cách tô màu---\n");
    for(i = 1; i <= num_nodes; i++){
        printf("Node %d - Color: %d    Style: %s\n", V[i].node, V[i].color, V[i].style);
    }
}

int main(){
    FILE *fptr = fopen("dothi.txt", "r");
    if(fptr == NULL){
        printf("Cannot open dothi.txt!\n");
        return -1;
    }

    int count[MAX] = {0};           //Array to store degree of nodes
    node_t V[MAX] = {0};               //Array to store {v1, v2, ..., vn} to mark nodes
    int num_nodes, num_edges;       //The number of nodes and edges of the graph    
    fscanf(fptr, "%d %d\n", &num_nodes, &num_edges);

    graphType graph[MAX];
    initialize(graph, num_nodes, num_edges);   
    input_graph(fptr, graph, num_nodes, num_edges);
    graph_degree(graph, count, num_edges);

    put_number(graph, count, V, num_nodes, num_edges);           //Create a {v1, v2, ..., vn} consequence to maximize greedy algorithm 
    greedy_algorithm(graph, count, V, num_nodes, num_edges);     //Coloring --> need <= k color

    print_result(graph, V, num_nodes, num_edges);
    fclose(fptr);
    return 0;
}