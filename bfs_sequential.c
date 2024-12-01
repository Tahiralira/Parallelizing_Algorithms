#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <omp.h>

#define NUM_NODES 1000000 // Adjusted to a million nodes
#define MAX_EDGES 50      // Maximum edges per node

typedef struct {
    int num_edges;
    int edges[MAX_EDGES];
} Node;

void create_graph(Node *nodes, int num_nodes);
void save_graph(Node *nodes, int num_nodes);
int load_graph(Node *nodes, int num_nodes, const char *filename);
void bfs(Node *nodes, int start_node, int num_nodes);

int main() {
    Node *nodes = (Node *)malloc(NUM_NODES * sizeof(Node));
    if (nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    const char *filename = "./data/input/graph_data.csv";
    if (!load_graph(nodes, NUM_NODES, filename)) {
        create_graph(nodes, NUM_NODES);
        save_graph(nodes, NUM_NODES);
    }

    printf("Starting BFS...\n");
    double start_time = omp_get_wtime();
    bfs(nodes, 0, NUM_NODES);  // Start BFS from node 0
    double end_time = omp_get_wtime();

    printf("BFS completed.\n");
    printf("Total time taken: %f seconds\n", end_time - start_time);

    free(nodes);
    return 0;
}

void create_graph(Node *nodes, int num_nodes) {
    srand(time(NULL));
    for (int i = 0; i < num_nodes; i++) {
        nodes[i].num_edges = rand() % MAX_EDGES;
        for (int j = 0; j < nodes[i].num_edges; j++) {
            do {
                nodes[i].edges[j] = rand() % num_nodes;
            } while (nodes[i].edges[j] == i); // Ensure no self-loop
        }
    }
    // Ensure at least one large connected component
    for (int i = 0; i < num_nodes - 1; i++) {
        if (nodes[i].num_edges < MAX_EDGES) {
            nodes[i].edges[nodes[i].num_edges++] = i + 1; // Ensure each node connects to the next
        }
    }
}

void save_graph(Node *nodes, int num_nodes) {
    FILE *fp = fopen("./data/input/graph_data.csv", "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file for writing\n");
        return;
    }
    for (int i = 0; i < num_nodes; i++) {
        fprintf(fp, "%d", nodes[i].num_edges);
        for (int j = 0; j < nodes[i].num_edges; j++) {
            fprintf(fp, ",%d", nodes[i].edges[j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

int load_graph(Node *nodes, int num_nodes, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("No existing graph file found. Creating new graph.\n");
        return 0; // File does not exist, return false
    }
    for (int i = 0; i < num_nodes; i++) {
        fscanf(fp, "%d", &nodes[i].num_edges);
        for (int j = 0; j < nodes[i].num_edges; j++) {
            fscanf(fp, ",%d", &nodes[i].edges[j]);
        }
    }
    fclose(fp);
    return 1; // Successfully loaded
}

void bfs(Node *nodes, int start_node, int num_nodes) {
    int *visited = (int *)calloc(num_nodes, sizeof(int));
    int *queue = (int *)malloc(num_nodes * sizeof(int));
    if (!visited || !queue) {
        fprintf(stderr, "Failed to allocate memory for BFS traversal.\n");
        free(visited);
        free(queue);
        exit(EXIT_FAILURE);
    }

    int front = 0, rear = 0;
    visited[start_node] = 1;
    queue[rear++] = start_node;

    while (front < rear) {
        int current_node = queue[front++];
        for (int i = 0; i < nodes[current_node].num_edges; i++) {
            int neighbor = nodes[current_node].edges[i];
            if (neighbor >= 0 && neighbor < num_nodes) {
                if (!visited[neighbor]) {
                    visited[neighbor] = 1;
                    queue[rear++] = neighbor;
                }
            } else {
                printf("Invalid neighbor index: %d at node %d\n", neighbor, current_node);
            }
        }
    }

    free(visited);
    free(queue);
}

