#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <unistd.h>  // Include for sleep and write operations

#define NUMBER_OF_POINTS 1000000 // Increased number of points to potentially extend execution time
#define D 10                    // Dimensions of data
#define K 20                    // Number of clusters
#define MAX_ITERATIONS 300      // Increased number of iterations

float *create_rand_data(int num_points) {
    float *data = (float *)malloc(num_points * D * sizeof(float));
    if (data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    FILE *fp;
    char filename[100];
    sprintf(filename, "./data/input/large_sequential_data_%d_%d.csv", num_points, D);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "File could not be opened\n");
        free(data);
        exit(1);
    }

    for (int i = 0; i < num_points * D; i++) {
        data[i] = (float)rand() / (float)RAND_MAX;
        fprintf(fp, "%f", data[i]);

        if ((i + 1) % D == 0 && i < num_points * D - 1) {
            fprintf(fp, "\n");
        } else {
            fprintf(fp, ",");
        }
    }

    fclose(fp);
    return data;
}

float distance(float *vec1, float *vec2, int dim) {
    float dist = 0.0;
    for (int i = 0; i < dim; i++) {
        dist += pow(vec1[i] - vec2[i], 2);
    }
    return sqrt(dist);
}

int assign_point(float *point, float *centroids, int k, int dim) {
    int cluster = 0;
    float minDist = INFINITY;
    for (int c = 0; c < k; c++) {
        float dist = distance(point, &centroids[c * dim], dim);
        if (dist < minDist) {
            minDist = dist;
            cluster = c;
        }
    }
    return cluster;
}

void update_centroid(float *sums, int *counts, float *centroids, int k, int dim) {
    for (int c = 0; c < k; c++) {
        if (counts[c] > 0) {
            for (int d = 0; d < dim; d++) {
                centroids[c * dim + d] = sums[c * dim + d] / counts[c];
            }
        }
    }
}

void print_centroids(float *centroids, int k, int dim) {
    printf("--------------------------CENTROIDS--------------------------\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: ", i);
        for (int j = 0; j < dim; j++) {
            printf("%f ", centroids[i * dim + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_progress(int iteration, int total) {
    const int barWidth = 50;
    float progress = (float)iteration / total;
    printf("[");
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %d%%\r", (int)(progress * 100));
    fflush(stdout);  // Flush to update the same line
}

int main() {
    srand(12345);

    float *points = create_rand_data(NUMBER_OF_POINTS * D);
    float *sums = calloc(K * D, sizeof(float));
    int *counts = calloc(K, sizeof(int));
    float *centroids = malloc(K * D * sizeof(float));
    if (sums == NULL || counts == NULL || centroids == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(points);
        free(sums);
        free(counts);
        free(centroids);
        exit(1);
    }

    // Initialize centroids with the first K points
    for (int i = 0; i < K * D; i++) {
        centroids[i] = points[i];
    }

    double start_time = omp_get_wtime();
    int iterations = 0;

    while (iterations < MAX_ITERATIONS) {
        memset(sums, 0, K * D * sizeof(float));
        memset(counts, 0, K * sizeof(int));

        for (int i = 0; i < NUMBER_OF_POINTS; i++) {
            int label = assign_point(&points[i * D], centroids, K, D);
            counts[label]++;
            for (int d = 0; d < D; d++) {
                sums[label * D + d] += points[i * D + d];
            }
        }

        update_centroid(sums, counts, centroids, K, D);
        print_progress(iterations + 1, MAX_ITERATIONS);  // Update progress
        iterations++;
    }

    double end_time = omp_get_wtime();
    printf("\n");  // Ensure the next print starts on a new line
    print_centroids(centroids, K, D);
    printf("Total iterations: %d\n", iterations);
    printf("Total time taken: %f seconds\n", end_time - start_time);

    free(points);
    free(sums);
    free(counts);
    free(centroids);

    return 0;
}
