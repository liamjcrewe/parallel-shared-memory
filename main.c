#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "debug.h"
#include "arrays.h"

typedef struct {
    double **values;
    int row;
    int col;
    double precision;
    int *threadAvailableFlag;
    pthread_mutex_t *threadAvailableFlagLock;
    int *valuesSolvedPoint;
    int *wereValuesModified;
} ThreadArgs;

int isEven(const int value)
{
    return value % 2 == 0;
}

void resetSolvedArray(int ** const solvedArray, const int dimension)
{
    int row, col;

    for(row = 0; row < dimension; row++) {
        for(col = 0; col < dimension; col++) {
            if (row == 0 || row == dimension - 1
                || col == 0 || col == dimension - 1
            ) {
                solvedArray[row][col] = 1;

                continue;
            }
            solvedArray[row][col] = 0;
        }
    }
}

void moveToNext(int * const row, int * const col, const int dimension)
{
    if (*col == dimension - 2) {
        *col = isEven(dimension) ? 1 : 2;
        *row += 1;

        return;
    }

    if (*col == dimension - 3) {
        *col = isEven(dimension) ? 2 : 1;
        *row += 1;

        return;
    }

    *col += 2;

    return;
}

int isLastEvenPoint(const int row, const int col, const int dimension)
{
    return (row == dimension - 2) && (col == dimension - 2);
}

int isLastOddPoint(const int row, const int col, const int dimension)
{
    return (row == dimension - 2) && (col == dimension - 3);
}

void *endThread(int * const threadAvailableFlag, pthread_mutex_t * const threadAvailableFlagLock)
{
    pthread_mutex_lock(threadAvailableFlagLock);
    *threadAvailableFlag = 1;
    pthread_mutex_unlock(threadAvailableFlagLock);

    return NULL;
}

void *updateValue(
    double **values,
    const int row,
    const int col,
    const double precision,
    int * const threadAvailableFlag,
    pthread_mutex_t * const threadAvailableFlagLock,
    int * const valuesSolvedPoint,
    int * const wereValuesModified
)
{
    double newValue = (values[row][col - 1] + values[row][col + 1]
                        + values[row - 1][col] + values[row + 1][col]) / 4;

    if (fabs(newValue - values[row][col]) < precision) {
        *valuesSolvedPoint = 1;

        return endThread(threadAvailableFlag, threadAvailableFlagLock);
    }

    *wereValuesModified = 1;
    values[row][col] = newValue;

    return endThread(threadAvailableFlag, threadAvailableFlagLock);
}

void *updateValueProxy(void *args)
{
    ThreadArgs *threadArgs = (ThreadArgs*) args;

    return updateValue(
        threadArgs->values,
        threadArgs->row,
        threadArgs->col,
        threadArgs->precision,
        threadArgs->threadAvailableFlag,
        threadArgs->threadAvailableFlagLock,
        threadArgs->valuesSolvedPoint,
        threadArgs->wereValuesModified
    );;
}

void solve(
    double ** const values,
    const int dimension,
    const int threads,
    const double precision
)
{
    int ** const valuesSolvedArray = createTwoDIntArray(dimension);
    resetSolvedArray(valuesSolvedArray, dimension);


    // Array of available threads, initially all are available
    int threadsAvailable[threads];
    for (int i = 0; i < threads; i++) {
        threadsAvailable[i] = 1;
    }

    // Array of locks on available threads
    pthread_mutex_t threadsAvailableLocks[threads];
    for (int i = 0; i < threads; i++) {
        pthread_mutex_init(&threadsAvailableLocks[i], NULL);
    }

    // Keep track of threads to join them later
    pthread_t tIds[threads];
    int tId;

    int row = 1;
    int col = 1;

    int rowColSum;

    // start with even indices
    int oddPointsFlag = 0;

    int wereValuesModified;

    while (twoDIntArrayContains(0, valuesSolvedArray, dimension)) {
        wereValuesModified = 0;

        /** Note: Acceptable race condition:
         *  --------------------------------
         *      If a thread is updating it's own ID in availableThreads to
         *      1 ('available'), this will miss this as it does not aquire the
         *      lock. However, this will simply loop and try again rather than
         *      have the overhead of the lock.
         *      The reverse will not happen, as setting the id in
         *      availableThreads to 0 ('unavailable') is done sequentially in
         *      this process.
         */
        tId = intArraySearch(1, threadsAvailable, threads);

        if (tId == -1) {
            // busy loop, may be CPU hungry but will be faster
            continue;
        }


        rowColSum = row + col;

        // Second condition:
        // reverse version of XOR
        if (!valuesSolvedArray[row][col]) {
            ThreadArgs args = {
                .values = values,
                .row = row,
                .col = col,
                .precision = precision,
                .threadAvailableFlag = &threadsAvailable[tId],
                .threadAvailableFlagLock = &threadsAvailableLocks[tId],
                .valuesSolvedPoint = &valuesSolvedArray[row][col],
                .wereValuesModified = &wereValuesModified
            };
            // May need to do it this way, depends what Balena allows.
            // args.values = values;
            // args.row = row;
            // args.col = col;
            // args.precision = precision;
            // args.threadsAvailableFlag = &threadsAvailable[tId];
            // args.threadsAvailableFlagLock = &threadsAvailableLocks[tId];
            // args.valuesSolvedPoint = &valuesSolvedArray[row][col];
            // args.wereValuesModified = &wereValuesModified;

            pthread_mutex_lock(&threadsAvailableLocks[tId]);

            // create the thread
            pthread_create(&tIds[tId], NULL, updateValueProxy, &args);
            // set lock to unavailable
            threadsAvailable[tId] = 0;

            pthread_mutex_unlock(&threadsAvailableLocks[tId]);
        }

        if (wereValuesModified) {
            resetSolvedArray(valuesSolvedArray, dimension);
        }

        if (isLastEvenPoint(row, col, dimension)
            || isLastOddPoint(row, col, dimension)
        ) {
            oddPointsFlag = oddPointsFlag ? 0 : 1;

            // Wait for all live threads to finish
            for (int i = 0; i < threads; i++) {
                if (!threadsAvailable[i]) {
                    continue;
                }

                pthread_join(tIds[i], NULL);
            }

            row = 1;
            col = 1 + oddPointsFlag;

            continue;
        }

        moveToNext(&row, &col, dimension);
    }

    // Make sure we wait for threads to finish
    for (int i = 0; i < threads; i++) {
        // Destroy all locks while we are here
        pthread_mutex_destroy(&threadsAvailableLocks[i]);

        if (!threadsAvailable[i]) {
            continue;
        }

        printf("Rogue thread found");
        pthread_join(tIds[i], NULL);
    }

    freeTwoDIntArray(valuesSolvedArray, dimension);
}

void fillWithRandomValues(double **input, int dimension)
{
    int row, col;
    srand(time(NULL));

    for (row = 0; row < dimension; row++) {
        for (col = 0; col < dimension; col++) {
            input[row][col] = rand() % 10;
        }
    }
}

int main(int args, char *argv[])
{
    // Input parameters
    const int dimension = 8;
    const int threads = 1;
    const double precision = 0.1;
    double ** const values = createTwoDDoubleArray(dimension);
    // Generate random input array
    fillWithRandomValues(values, dimension);

    // Log input
    printf("Input:\n");
    print2dDoubleArray(values, dimension);

    // Solve and update values
    solve(values, dimension, threads, precision);

    // Log solution
    printf("\nSolution:\n");
    print2dDoubleArray(values, dimension);

    // Free memory
    freeTwoDDoubleArray(values, dimension);

    return 0;
}
