#include <stdio.h>

/**
 * Print a two dimensional array of doubles to the screen.
 *
 * @param array     The array to print
 * @param dimension The dimension of the array to print
 */
void write2dDoubleArray(
    FILE * const f,
    double ** const array,
    const int dimension
)
{
    for (int row = 0; row < dimension; ++row) {
        for (int col = 0; col < dimension; ++col) {
            fprintf(f, "%10f ", array[row][col]);
        }
        fputs("\n", f);
    }
}
