// Berlin-bus-analyzer program using C programming language
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define MAX_NODES 15000
#define INF INT_MAX

// Common structures
struct Edge {
    int node;
    int distance;
    float time;
    struct Edge* next;
};

struct Edge* adj[MAX_NODES] = {NULL};
int nodes[MAX_NODES];
int numNodes = 0;
int visited[MAX_NODES] = {0};
int parent[MAX_NODES];

// Performance tracking variables
int comparisons = 0;
int relaxations = 0;
int parent_changes = 0;
int auxiliary_size = 0;

// For output.csv
int path_distances[MAX_NODES][MAX_NODES];
int path_parents[MAX_NODES][MAX_NODES];

// Function prototypes
void createAdjacencyList();
void performDijkstra();
void printPathToFile(FILE *file, int node);
void writeOutputCSV(int startNode, int trial);
void dijkstra(int startNode, int trial);
void addEdge(int u, int v, int distance, float time);
int isNodeExists(int node);
void parseCSV(const char *filename);
void freeMemory();

int main() {
    int choice;
    
    do {
        printf("\n===== Berlin Bus Network Analyzer =====\n\n");
        printf("1. Create adjacency list from CSV\n");
        printf("2. Perform Dijkstra's algorithm\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        switch(choice) {
            case 1:
                createAdjacencyList();
                break;
            case 2:
                performDijkstra();
                break;
            case 3:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 3);
    
    return 0;
}

void createAdjacencyList() {
    FILE *file = fopen("berlin_network_bus.csv", "r");
    if (!file) {
        printf("Error opening file 'berlin_network_bus.csv'\n");
        return;
    }

    int start, stop, dist;
    float time;
    char buffer[512];
    printf("\nLoading edges from CSV file...\n");

    // Skip header line
    if (!fgets(buffer, sizeof(buffer), file)) {
        printf("Error reading header line\n");
        fclose(file);
        return;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        char *duration_ptr = NULL;
        char *d_ptr = NULL;

        // Extract first two integers (start and stop nodes)
        if (sscanf(buffer, "%d;%d;", &start, &stop) != 2) {
            printf("Warning: Skipping invalid line (start/stop nodes)\n");
            continue;
        }

        // Find 'duration_avg' value
        for (char *p = buffer; *p; p++) {
            if (strncmp(p, "'duration_avg':", 15) == 0) {
                duration_ptr = p + 15;
                break;
            }
        }

        // Find 'd' value (distance)
        for (char *p = buffer; *p; p++) {
            if (strncmp(p, "'d':", 4) == 0) {
                d_ptr = p + 4;
                break;
            }
        }

        // Parse duration_avg (time) and distance (dist)
        if (duration_ptr && d_ptr) {
            if (sscanf(duration_ptr, "%f", &time) == 1 && 
                sscanf(d_ptr, "%d", &dist) == 1) {
                addEdge(start, stop, dist, time);
            } else {
                printf("Warning: Failed to parse time/distance for nodes %d->%d\n", start, stop);
            }
        } else {
            printf("Warning: Missing time/distance data for nodes %d->%d\n", start, stop);
        }
    }

    fclose(file);
    printf("File loaded successfully.\n", ftell(file));

    // Write adjacency list to file
    FILE *file_output = fopen("berlin_list_bus.csv", "w");
    if (!file_output) {
        printf("Error opening output file 'berlin_list_bus.csv'\n");
        return;
    }

    printf("\nWriting adjacency list to file...\n");

    for (int i = 0; i < MAX_NODES; i++) {
        if (adj[i] != NULL) {
            fprintf(file_output, "%d", i);

            struct Edge* temp = adj[i];
            while (temp) {
                fprintf(file_output, ";[%d,%d,%.2f]", temp->node, temp->distance, temp->time);
                temp = temp->next;
            }
            fprintf(file_output, "\n");
        }
    }

    fclose(file_output);
    printf("Adjacency list successfully written to 'berlin_list_bus.csv'\n");
    
    // Free memory after writing to file
    freeMemory();
}

void performDijkstra() {
    // First parse the adjacency list file
    parseCSV("berlin_list_bus.csv");
    
    if (numNodes == 0) {
        printf("No nodes found in the adjacency list. Please create the adjacency list first.\n");
        return;
    }
    
    int startNode;
    printf("Enter start node: ");
    if (scanf("%d", &startNode) != 1) {
        printf("Invalid input\n");
        freeMemory();
        return;
    }
    
    if (!isNodeExists(startNode)) {
        printf("Start node %d not found in the network\n", startNode);
        freeMemory();
        return;
    }
    
    int trials;
    printf("Enter number of trials: ");
    if (scanf("%d", &trials) != 1) {
        printf("Invalid input\n");
        freeMemory();
        return;
    }
    
    for (int trial = 1; trial <= trials; trial++) {
        dijkstra(startNode, trial);
    }
    
    printf("Shortest paths have been saved to berlin_path.csv\n");
    printf("Performance metrics have been saved to performance.csv\n");
    printf("Path details have been saved to output.csv\n");
    
    freeMemory();
}

void printPathToFile(FILE *file, int node) {
    if (node == -1) return;
    printPathToFile(file, parent[node]);
    if (parent[node] != -1) {
        fprintf(file, "->");
    }
    fprintf(file, "%d", node);
}

void writeOutputCSV(int startNode, int trial) {
    FILE *output_file = fopen("output.csv", trial == 1 ? "w" : "a");
    if (!output_file) {
        printf("Error opening output file\n");
        exit(0);
    }

    if (trial == 1) {
        fprintf(output_file, "start node,trial,distance,parent\n");
    }

    for (int i = 0; i < numNodes; i++) {
        int node = nodes[i];
        if (node == startNode) continue;

        // Build distance string
        char dist_str[1024] = "{";
        int current = node;
        int count = 0;
        int distances[MAX_NODES];
        
        // Get distances in reverse order
        while (current != startNode && current != -1 && count < MAX_NODES) {
            distances[count++] = path_distances[trial-1][current];
            current = path_parents[trial-1][current];
        }
        distances[count++] = 0; // Start node distance
        
        // Add distances in correct order
        for (int j = count-1; j >= 0; j--) {
            char temp[20];
            if (j == count-1) {
                sprintf(temp, "%d", distances[j]);
            } else {
                sprintf(temp, ", %d", distances[j]);
            }
            strcat(dist_str, temp);
        }
        strcat(dist_str, "}");

        // Build parent string
        char parent_str[1024] = "{";
        current = node;
        count = 0;
        int parents[MAX_NODES];
        
        // Get parents in reverse order
        while (current != startNode && current != -1 && count < MAX_NODES) {
            parents[count++] = path_parents[trial-1][current];
            current = path_parents[trial-1][current];
        }
        parents[count++] = -1; // Start node has no parent
        
        // Add parents in correct order
        for (int j = count-1; j >= 0; j--) {
            char temp[50];
            if (j == count-1) {
                if (parents[j] == -1) {
                    sprintf(temp, "[]");
                } else {
                    sprintf(temp, "[%d]", parents[j]);
                }
            } else {
                if (parents[j] == -1) {
                    sprintf(temp, ", []");
                } else {
                    sprintf(temp, ", [%d]", parents[j]);
                }
            }
            strcat(parent_str, temp);
        }
        strcat(parent_str, "}");

        // Write to file (using simple trial number)
        fprintf(output_file, "%d,%d,\"%s\",\"%s\"\n", 
                startNode, trial, dist_str, parent_str);
    }

    fclose(output_file);
}

void dijkstra(int startNode, int trial) {
    int dist[MAX_NODES];
    float time[MAX_NODES];
    clock_t start_time, end_time;
    
    // Reset performance counters
    comparisons = 0;
    relaxations = 0;
    parent_changes = 0;
    auxiliary_size = 0;
    
    for (int i = 0; i < MAX_NODES; i++) {
        dist[i] = INF;
        time[i] = INF;
        parent[i] = -1;
        visited[i] = 0;
    }
    
    start_time = clock();
    
    dist[startNode] = 0;
    time[startNode] = 0.0;
    
    while (1) {
        // Find unvisited node with smallest distance
        int u = -1;
        int minDist = INF;
        float minTime = INF;
        
        for (int i = 0; i < numNodes; i++) {
            int node = nodes[i];
            comparisons++;
            if (!visited[node] && (dist[node] < minDist || 
                (dist[node] == minDist && time[node] < minTime))) {
                u = node;
                minDist = dist[node];
                minTime = time[node];
            }
        }
        
        if (u == -1) break;
        visited[u] = 1;
        
        struct Edge* edge = adj[u];
        while (edge) {
            int v = edge->node;
            int weight = edge->distance;
            float travelTime = edge->time;
            
            comparisons += 2;
            if (dist[u] + weight < dist[v] || 
                (dist[u] + weight == dist[v] && time[u] + travelTime < time[v])) {
                relaxations++;
                if (parent[v] != u) parent_changes++;
                dist[v] = dist[u] + weight;
                time[v] = time[u] + travelTime;
                parent[v] = u;
            }
            edge = edge->next;
        }
    }
    
    end_time = clock();
    
    auxiliary_size = numNodes; // For simple implementation
    
    // Store results for this trial
    for (int i = 0; i < numNodes; i++) {
        int node = nodes[i];
        path_distances[trial-1][node] = dist[node];
        path_parents[trial-1][node] = parent[node];
    }
    
    // Write results to berlin_path.csv (original functionality)
    FILE *file = fopen("berlin_path.csv", "w");
    if (!file) {
        printf("Error opening output file\n");
        exit(0);
    }
    
    fprintf(file, "Start Node;End Node;Path;Distance;Time\n");
    
    for (int i = 0; i < numNodes; i++) {
        int node = nodes[i];
        if (node == startNode) continue;
        
        fprintf(file, "%d;%d;", startNode, node);
        if (dist[node] == INF) {
            fprintf(file, "No path;;\n");
        } else {
            printPathToFile(file, node);
            fprintf(file, ";%d;%.2f\n", dist[node], time[node]);
        }
    }
    fclose(file);
    
    // Write performance metrics to performance.csv
    FILE *perf_file = fopen("performance.csv", trial == 1 ? "w" : "a");
    if (!perf_file) {
        printf("Error opening performance file\n");
        exit(0);
    }
    
    if (trial == 1) {
        fprintf(perf_file, "Start Node,Trial,Execution Time (ms),Comparisons,Relaxations,Auxiliary Size,Avg Parent Changes\n");
    }
    
    double exec_time = ((double)(end_time - start_time)) * 1000 / CLOCKS_PER_SEC;
    float avg_parent_changes = relaxations > 0 ? (float)parent_changes/relaxations : 0;
    
    fprintf(perf_file, "%d,%d,%.2f,%d,%d,%d,%.2f\n",
            startNode, trial, exec_time, comparisons, 
            relaxations, auxiliary_size, avg_parent_changes);
    
    fclose(perf_file);
    
    // Write to output.csv with the specific format
    writeOutputCSV(startNode, trial);
}

void addEdge(int u, int v, int distance, float time) {
    struct Edge* current = adj[u];
    while (current) {
        if (current->node == v) {
            if (distance < current->distance || 
                (distance == current->distance && time < current->time)) {
                current->distance = distance;
                current->time = time;
            }
            return;
        }
        current = current->next;
    }
    
    struct Edge* newEdge = (struct Edge*)malloc(sizeof(struct Edge));
    if (!newEdge) {
        printf("Memory allocation failed\n");
        exit(0);
    }
    newEdge->node = v;
    newEdge->distance = distance;
    newEdge->time = time;
    newEdge->next = adj[u];
    adj[u] = newEdge;
}

int isNodeExists(int node) {
    for (int i = 0; i < numNodes; i++) {
        if (nodes[i] == node) {
            return 1;
        }
    }
    return 0;
}

void parseCSV(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("File opening failed\n");
        exit(0);
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ";");
        if (!token) continue;
        
        int node = atoi(token);
        if (!isNodeExists(node)) {
            nodes[numNodes++] = node;
        }
        
        while ((token = strtok(NULL, ";")) != NULL) {
            int neighbor, distance;
            float timeVal;
            
            if (sscanf(token, "[%d,%d,%f]", &neighbor, &distance, &timeVal) == 3) {
                addEdge(node, neighbor, distance, timeVal);
                
                if (!isNodeExists(neighbor)) {
                    nodes[numNodes++] = neighbor;
                }
            }
        }
    }
    fclose(file);
}

void freeMemory() {
    for (int i = 0; i < MAX_NODES; i++) {
        struct Edge* current = adj[i];
        while (current) {
            struct Edge* temp = current;
            current = current->next;
            free(temp);
        }
        adj[i] = NULL;
    }
}
