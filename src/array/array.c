#include <stdlib.h>

/**
 * Create a square two dimensional array of ints of the dimension specified.
 * Uses malloc, should always be followed later in the calling code with
 * freeTwoDIntArray.
 *
 * @param  dimension The dimension of the two dimensional array to create
 *
 * @return           Pointer to the created two dimensional array
 */
int **createTwoDIntArray(const int dimension)
{
    int **rows = (int **)malloc(dimension * sizeof(int*));

    for (int row = 0; row < dimension; row++) {
        rows[row] = (int *)malloc(dimension * sizeof(int));
    }

    return rows;
}

/**
 * Create a square two dimensional array of doubles of the dimension specified.
 * Uses malloc, should always be followed later in the calling code with
 * freeTwoDDoubleArray.
 *
 * @param  dimension The dimension of the two dimensional array to create
 *
 * @return           Pointer to the created two dimensional array
 */
double **createTwoDDoubleArray(const int dimension)
{
    double **rows = (double **)malloc(dimension * sizeof(double*));

    for (int row = 0; row < dimension; row++) {
        rows[row] = (double *)malloc(dimension * sizeof(double));
    }

    return rows;
}

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 *
 * @param array     The two dimensional array to free
 * @param dimension The dimension of the two dimensional array to free
 */
void freeTwoDIntArray(int **array, const int dimension)
{
    for(int row = 0; row < dimension; row++) {
        free(array[row]);
    }

    free(array);
}

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 *
 * @param array     The two dimensional array to free
 * @param dimension The dimension of the two dimensional array to free
 */
void freeTwoDDoubleArray(double **array, const int dimension)
{
    for(int row = 0; row < dimension; row++) {
        free(array[row]);
    }

    free(array);
}

/**
 * Check if the given two dimensional array of ints contains the given value.
 *
 * @param  value     The value to search for
 * @param  array     The two dimensional array to search in
 * @param  dimension The dimension of the two dimensional array to search
 *
 * @return           1 if value found, 0 otherwise
 */
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

/**
 * Simple array search on an array of integers. Returns the index of the first
 * occurence of the value if it is found, or -1 otherwise.
 *
 * @param  value     The value to search for
 * @param  array     The array to search in
 * @param  dimension The dimension of the array to search in
 *
 * @return           Index of first occurence of value if found, -1 otherwise
 */
int intArraySearch(const int value, int * const array, const int dimension)
{
    for (int i = 0; i < dimension; i++) {
        if (value == array[i]) {
            return i;
        }
    }

    return -1;
}
