#include <stdlib.h>

int **createTwoDIntArray(int dimension)
{
    int **rows = (int **)malloc(dimension * sizeof(int*));

    int row;

    for (row = 0; row < dimension; row++) {
        rows[row] = (int *)malloc(dimension * sizeof(int));
    }

    return rows;
}

double **createTwoDDoubleArray(int dimension)
{
    double **rows = (double **)malloc(dimension * sizeof(double*));

    int row;

    for (row = 0; row < dimension; row++) {
        rows[row] = (double *)malloc(dimension * sizeof(double));
    }

    return rows;
}

int twoDIntArrayContains(int value, int **array, int dimension)
{
    int row, col;

    for(row = 0; row < dimension; row++) {
        for(col = 0; col < dimension; col++) {
            if (array[row][col] == value) {
                return 1;
            }
        }
    }

    return 0;
}

void freeTwoDIntArray(int **array, int dimension)
{
    int row;

    for(row = 0; row < dimension; row++) {
        free(array[row]);
    }

    free(array);
}

void freeTwoDDoubleArray(double **array, int dimension)
{
    int row;

    for(row = 0; row < dimension; row++) {
        free(array[row]);
    }

    free(array);
}
