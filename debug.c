#include <stdio.h>

void print2dArray(int ** const array, const int dimension)
{
    int row, col;

    for (row = 0; row < dimension; row++) {
        for (col = 0; col < dimension; col++) {
            printf("%10d ", array[row][col]);
        }
        puts("");
    }
}

void print2dDoubleArray(double ** const array, const int dimension)
{
    int row, col;

    for (row = 0; row < dimension; ++row) {
        for (col = 0; col < dimension; ++col) {
            printf("%10f ", array[row][col]);
        }
        puts("");
    }
}
