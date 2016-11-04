/**
 * Create a square two dimensional array of ints of the dimension specified.
 *
 * @param  dimension The dimension of the two dimensional array to create
 *
 * @return           Pointer to the created two dimensional array
 */
int **createTwoDIntArray(const int dimension);

/**
 * Create a square two dimensional array of doubles of the dimension specified.
 *
 * @param  dimension The dimension of the two dimensional array to create
 *
 * @return           Pointer to the created two dimensional array
 */
double **createTwoDDoubleArray(const int dimension);

/**
 * Note: It would be nice to do the above two in one function as they are
 *       almost identical, but I'm not sure how to do this in C, or if it is
 *       even a good idea.
 */

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 *
 * @param array     The two dimensional array to free
 * @param dimension The dimension of the two dimensional array to free
 */
void freeTwoDIntArray(int **array, const int dimension);

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 *
 * @param array     The two dimensional array to free
 * @param dimension The dimension of the two dimensional array to free
 */
void freeTwoDDoubleArray(double **array, const int dimension);

/**
 * Note: As above, these two free functions are almost identical.
 */

/**
 * Check if the given two dimensional array of ints contains the given value.
 *
 * @param  value     The value to search for
 * @param  array     The two dimensional array to search in
 * @param  dimension The dimension of the two dimensional array to search in
 *
 * @return           1 if value found, 0 otherwise
 */
int twoDIntArrayContains(
    const int value,
    int ** const array,
    const int dimension
);

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
int intArraySearch(const int value, int * const array, const int dimension);
