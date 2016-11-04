#include <stdlib.h>

int **createTwoDIntArray(const int dimension)
{
    int **rows = (int **)malloc(dimension * sizeof(int*));

    for (int row = 0; row < dimension; row++) {
        rows[row] = (int *)malloc(dimension * sizeof(int));
    }

    return rows;
}

double **createTwoDDoubleArray(const int dimension)
{
    double **rows = (double **)malloc(dimension * sizeof(double*));

    for (int row = 0; row < dimension; row++) {
        rows[row] = (double *)malloc(dimension * sizeof(double));
    }

    return rows;
}

int twoDIntArrayContains(
    const int value,
    int ** const array,
    const int dimension
)
{
    for(int row = 0; row < dimension; row++) {
        for(int col = 0; col < dimension; col++) {
            if (array[row][col] == value) {
                return 1;
            }
        }
    }

    return 0;
}

void freeTwoDIntArray(int **array, const int dimension)
{
    for(int row = 0; row < dimension; row++) {
        free(array[row]);
    }

    free(array);
}

void freeTwoDDoubleArray(double **array, const int dimension)
{
    for(int row = 0; row < dimension; row++) {
        free(array[row]);
    }

    free(array);
}

int intArraySearch(const int value, int * const array, const int dimension)
{
    for (int i = 0; i < dimension; i++) {
        if (value == array[i]) {
            return i;
        }
    }

    return -1;
}
