#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define PIPE_NAME "mypipe"

void readMatrix(int **matrix, int rows, int cols, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                fprintf(stderr, "Error reading matrix from file %s\n", filename);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(file);
}

void printMatrix(int **matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void addMatrices(int **matrix1, int **matrix2, int **result, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = matrix1[i][j] + matrix2[i][j];
        }
    }
}

void subtractMatrices(int **matrix1, int **matrix2, int **result, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = matrix1[i][j] - matrix2[i][j];
        }
    }
}

void multiplyMatrices(int **matrix1, int **matrix2, int **result, int rows, int cols, int inner_dim) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = 0;
            for (int k = 0; k < inner_dim; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
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
    int num_files, rows, cols, choice;

    printf("Enter the number of input files: ");
    scanf("%d", &num_files);

    printf("---------------------------------\n");
    printf("Enter the number of rows: ");
    scanf("%d", &rows);
    printf("Enter the number of columns: ");
    scanf("%d", &cols);

    printf("---------------------------------\n");
    do {
        printf("Choose the operation:\n");
        printf("1. Add\n");
        printf("2. Subtract\n");
        printf("3. Multiply\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice < 1 || choice > 3) {
            printf("Invalid choice. Please select a valid operation.\n");
        }
    } while (choice < 1 || choice > 3);
    printf("---------------------------------\n");
    
    int ***matrices = (int ***)malloc(num_files * sizeof(int **));
    for (int i = 0; i < num_files; i++) {
        matrices[i] = allocateMatrix(rows, cols);
    }

    for (int i = 0; i < num_files; i++) {
        char filename[100];
        printf("Enter the name of file %d: ", i + 1);
        scanf("%s", filename);

        readMatrix(matrices[i], rows, cols, filename);
    }
    int **result = allocateMatrix(rows, cols);
    switch (choice) {
        case 1:
            // Cộng các ma trận
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    result[i][j] = matrices[0][i][j];
                }
            }
            for (int i = 1; i < num_files; i++) {
                addMatrices(result, matrices[i], result, rows, cols);
            }
            break;
        case 2:
            // Trừ các ma trận
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    result[i][j] = matrices[0][i][j];
                }
            }
            for (int i = 1; i < num_files; i++) {
                subtractMatrices(result, matrices[i], result, rows, cols);
            }
            break;
        case 3:
            // Nhân các ma trận
            if (num_files < 2 || rows != cols) {
                fprintf(stderr, "Error: Matrix multiplication requires at least two square matrices.\n");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    result[i][j] = matrices[0][i][j];
                }
            }
            for (int i = 1; i < num_files; i++) {
                int **temp = allocateMatrix(rows, cols);
                multiplyMatrices(result, matrices[i], temp, rows, cols, cols);
                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < cols; j++) {
                        result[i][j] = temp[i][j];
                    }
                }
                freeMatrix(temp, rows);
            }
            break;
        default:
            fprintf(stderr, "Invalid choice.\n");
            exit(EXIT_FAILURE);
    }

    // Write result to named pipe
    mkfifo(PIPE_NAME, 0666); // Create the named pipe
    int fd = open(PIPE_NAME, O_WRONLY);
    if (fd < 0) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    // Send dimensions and data
    write(fd, &rows, sizeof(int));
    write(fd, &cols, sizeof(int));
    for (int i = 0; i < rows; i++) {
        write(fd, result[i], cols * sizeof(int));
    }

    close(fd);
    
    printf("---------------------------------\n");
    printf("Data has been successfully sent to the consumer.\n");

    for (int i = 0; i < num_files; i++) {
        freeMatrix(matrices[i], rows);
    }
    free(matrices);
    freeMatrix(result, rows);

    return 0;
}
