int **createTwoDIntArray(const int dimension);
double **createTwoDDoubleArray(const int dimension);
void freeTwoDIntArray(int **array, const int dimension);
void freeTwoDDoubleArray(double **array, const int dimension);
int twoDIntArrayContains(
    const int value,
    int ** const array,
    const int dimension
);
int intArraySearch(const int value, int * const array, const int dimension);
