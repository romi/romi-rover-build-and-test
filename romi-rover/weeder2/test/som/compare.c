#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct _matrix_t {
        char name[32];
        int32_t rows;
        int32_t cols;
        double values[100000];
} matrix_t;

int matrix_read(matrix_t *matrix, int fd)
{
        ssize_t n;
        size_t len;
        
        n = read(fd, (void *) &len, sizeof(int32_t));
        if (n != sizeof(int32_t)) {
                printf("Read failed\n");
                exit(1);
        }
        if (len < 1 || len > 31) {
                printf("Invalid name length: %lu\n", len);
                exit(1);
        }
        
        n = read(fd, (void *) matrix->name, len);
        if (n != (ssize_t)len) {
                printf("Read failed\n");
                exit(1);
        }
        matrix->name[len] = 0;

        n = read(fd, (void *) &matrix->rows, sizeof(matrix->rows));
        if (n != sizeof(matrix->rows)) {
                printf("Read failed\n");
                exit(1);
        }
        if (matrix->rows < 0 || matrix->rows > 100000) {
                printf("Invalid number of rows: %d\n", matrix->rows);
                exit(1);
        }

        n = read(fd, (void *) &matrix->cols, sizeof(matrix->cols));
        if (n != sizeof(matrix->cols)) {
                printf("Read failed\n");
                exit(1);
        }
        if (matrix->cols < 0 || matrix->cols > 100000) {
                printf("Invalid number of columns: %d\n", matrix->cols);
                exit(1);
        }

        len = (size_t) (matrix->rows * matrix->cols);
        if (len > 100000) {
                printf("Matrix too large (%lu)\n", len);
                exit(1);
        }
        
        size_t size = ( len * sizeof(double));
        
        n = read(fd, (void *) matrix->values, size);                
        if (n != (ssize_t)size) {
                printf("Read failed\n");
                exit(1);
        }

        return 0;
}

void matrix_print(matrix_t *m)
{
        int index = 0;
        printf("==== %s[%d][%d] ====\n", m->name, m->rows, m->cols);
        for (int row = 0; row < m->rows; row++) { 
                for (int col = 0; col < m->cols; col++) {
                        printf("[%d,%d]=%.24f\n", row, col, m->values[index]);
                        index++;
                }
        }
}

void matrix_print_two(matrix_t *m1, matrix_t *m2)
{
        int index = 0;
        printf("==== %s[%d][%d] vs %s[%d][%d] ====\n",
               m1->name, m1->rows, m1->cols,
               m2->name, m2->rows, m2->cols);
        for (int row = 0; row < m1->rows; row++) { 
                for (int col = 0; col < m1->cols; col++) {
                        printf("[%d,%d]\t%.24f\t%.24f\n",
                               row, col,
                               m1->values[index],
                               m2->values[index]);
                        index++;
                }
        }
}

int compare(int matrix_number, int fd1, int fd2)
{
        matrix_t matrix1;
        matrix_t matrix2;
        
        int err1 = matrix_read(&matrix1, fd1);
        int err2 = matrix_read(&matrix2, fd2);

        if (err1 || err2) {
                exit(1);
        }

        if (matrix1.rows != matrix2.rows) {
                printf("Compare %d: row count doesn't match\n", matrix_number);
                exit(1);
        }

        if (matrix1.cols != matrix2.cols) {
                printf("Compare %d: column count doesn't match\n", matrix_number);
                exit(1);
        }

        int index = 0;
        for (int row = 0; row < matrix1.rows; row++) { 
                for (int col = 0; col < matrix1.cols; col++) {
                        if (matrix1.values[index] != matrix2.values[index]) { 
                                printf("Compare %d: %s - %s: values[%d,%d] don't match: %.24f != %.24f\n",
                                       matrix_number,
                                       matrix1.name,
                                       matrix2.name,
                                       row, col,
                                       matrix1.values[index],
                                       matrix2.values[index]);
                                matrix_print_two(&matrix1, &matrix2);
                                exit(1);
                        }
                        index++;
                }
        }

        printf("Compare %d: %s - %s: OK\n",
               matrix_number,
               matrix1.name,
               matrix2.name);
        
        return 0;
}

int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("Usage: compare <file1> <file2>\n");
                exit(1);
        }

        int fd1 = open(argv[1], O_RDONLY);
        if (fd1 == -1) {
                perror(argv[1]);
                exit(1);
        }

        int fd2 = open(argv[2], O_RDONLY);
        if (fd2 == -1) {
                perror(argv[2]);
                exit(1);
        }
        
        for (int i = 0; i < 10000; i++)
                compare(i, fd1, fd2);

        return 0;
}

// gcc -g -O0 compare.c -o compare
