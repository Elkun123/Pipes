#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> // For O_RDONLY
#include <sys/stat.h> // For mkfifo
#include <sys/types.h>
#include <time.h> // For time()

#define PIPE_NAME "mypipe"

void printMatrix(int **matrix, int rows, int cols, FILE *file) {
    // Print to terminal and also write to file
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
            fprintf(file, "%d ", matrix[i][j]); // Write to file
        }
        printf("\n");
        fprintf(file, "\n"); // New line in file
    }
}

int **allocateMatrix(int rows, int cols) {
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }
    return matrix;
}

void freeMatrix(int **matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main() {
    int rows, cols;

    // Open the named pipe
    mkfifo(PIPE_NAME, 0666);
    int fd = open(PIPE_NAME, O_RDONLY);
    if (fd < 0) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    // Read dimensions
    read(fd, &rows, sizeof(int));
    read(fd, &cols, sizeof(int));

    int **result = allocateMatrix(rows, cols);

    // Read matrix data
    for (int i = 0; i < rows; i++) {
        read(fd, result[i], cols * sizeof(int));
    }

    close(fd);

    // Print the result to terminal and save to file
    FILE *outputFile = fopen("output.txt", "a"); // Open file in append mode
    if (outputFile == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Get current time and print it
    time_t currentTime = time(NULL);
    struct tm *tmInfo = localtime(&currentTime);
    char timeString[100];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", tmInfo);
    fprintf(outputFile, "Time of save: %s\n", timeString); // Write timestamp to file

    // Print the matrix to both terminal and file
    printf("Received data from product:\n");
    printMatrix(result, rows, cols, outputFile);

    fclose(outputFile);

    freeMatrix(result, rows);

    return 0;
}
