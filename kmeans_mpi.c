#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#define NUMBER_OF_POINTS 1000000 // Number of data points
#define D 10                     // Dimensionality of data
#define K 20                     // Number of clusters
#define MAX_ITERATIONS 300       // Maximum number of iterations

// Function prototypes
float *create_rand_data(int num_points, int rank, int num_procs);
float distance(float *vec1, float *vec2, int dim);
void kmeans(float *points, int num_points, float *centroids, int rank, int num_procs);

int main(int argc, char *argv[]) {
    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Each process will generate its portion of random data
    int points_per_proc = NUMBER_OF_POINTS / num_procs;
    float *points = create_rand_data(points_per_proc * D, rank, num_procs);

    float *centroids = NULL;
    if (rank == 0) {
        centroids = malloc(K * D * sizeof(float)); // Only root initializes centroids
        for (int i = 0; i < K * D; i++) {
            centroids[i] = points[i]; // Initialize centroids with the first K points
        }
    }

    double start_time = MPI_Wtime();
    kmeans(points, points_per_proc, centroids, rank, num_procs);
    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Total time taken: %f seconds\n", end_time - start_time);
        free(centroids);
    }
    free(points);

    MPI_Finalize();
    return 0;
}

float *create_rand_data(int num_points, int rank, int num_procs) {
    srand(12345 + rank); // Ensure different seeds for different processes
    float *data = (float *)malloc(num_points * sizeof(float));
    for (int i = 0; i < num_points; i++) {
        data[i] = (float)rand() / (float)RAND_MAX;
    }
    return data;
}

float distance(float *vec1, float *vec2, int dim) {
    float dist = 0.0;
    for (int i = 0; i < dim; i++) {
        dist += pow(vec1[i] - vec2[i], 2);
    }
    return sqrt(dist);
}

void kmeans(float *points, int num_points, float *centroids, int rank, int num_procs) {
    float *local_sums = calloc(K * D, sizeof(float));
    int *local_counts = calloc(K, sizeof(int));
    float *global_sums = NULL;
    int *global_counts = NULL;

    if (rank == 0) {
        global_sums = calloc(K * D, sizeof(float));
        global_counts = calloc(K, sizeof(int));
    }

    int iterations = 0;
    while (iterations < MAX_ITERATIONS) {
        memset(local_sums, 0, K * D * sizeof(float));
        memset(local_counts, 0, K * sizeof(int));

        // Assign points to the nearest centroid
        for (int i = 0; i < num_points; i++) {
            int closest = 0;
            float min_dist = INFINITY;
            for (int c = 0; c < K; c++) {
                float dist = distance(&points[i * D], &centroids[c * D], D);
                if (dist < min_dist) {
                    closest = c;
                    min_dist = dist;
                }
            }
            local_counts[closest]++;
            for (int d = 0; d < D; d++) {
                local_sums[closest * D + d] += points[i * D + d];
            }
        }

        // Reduce local sums and counts to global sums and counts on root
        MPI_Reduce(local_sums, global_sums, K * D, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(local_counts, global_counts, K, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Update centroids on root
        if (rank == 0) {
            for (int i = 0; i < K; i++) {
                if (global_counts[i] > 0) {
                    for (int d = 0; d < D; d++) {
                        centroids[i * D + d] = global_sums[i * D + d] / global_counts[i];
                    }
                }
            }
            MPI_Bcast(centroids, K * D, MPI_FLOAT, 0, MPI_COMM_WORLD);
        } else {
            MPI_Bcast(centroids, K * D, MPI_FLOAT, 0, MPI_COMM_WORLD);
        }

        iterations++;
    }

    if (rank == 0) {
        free(global_sums);
        free(global_counts);
    }
    free(local_sums);
    free(local_counts);
}
