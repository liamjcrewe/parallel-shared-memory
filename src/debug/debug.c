#include <stdio.h>

void print2dDoubleArray(double ** const array, const int dimension)
{
    for (int row = 0; row < dimension; ++row) {
        for (int col = 0; col < dimension; ++col) {
            printf("%10f ", array[row][col]);
        }
        puts("");
    }
}
