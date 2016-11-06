/**
 * Algorithm background:
 * ---------------------
 *
 * This file contains most of the complicated logic to solve the problem using
 * a parallel algorithm. It does, in places, sacrifice code readability for
 * speed and efficiency, hence this long comment. To give some background to
 * the algorithm, the input two dimensional array should be viewed as follows:
 *
 *   X X X X X          X X X X X X
 *   X E O E X          X E O E O X
 *   X O E O X    AND   X O E O E X
 *   X E O E X          X E O E O X
 *   X X X X X          X O E O E X
 *                      X X X X X X
 *
 * where E = 'even' point - where row and column indices add to an even value,
 *       O = 'odd' point - where row and column indicies add to an odd value,
 *       X = 'edge' point - a fixed point.
 *
 * Using this view it is clear that all Es can be worked on in parallel, and
 * likewise for all Os. This is the basis of the parallel algorithm; work on as
 * many points that are independent of one another as possible.
 *
 * Much of the complex logic is in finding out which point give to the next
 * thread to work on. See moveToNext function comment for further details on
 * this.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../array/array.h"
#include "../utility/utility.h"

// struct to pass multiple arguments to thread callback function
typedef struct {
    double ** const values; // The two dimensional array of values being solved
    const int row; // The row to update in the thread
    const int col; // The column to update in the thread
    const double precision; // The precision to work to
    int * const threadAvailableFlag; // Flag - is a given thread available
    pthread_mutex_t * const threadAvailableFlagLock; // Lock on availability
                                                     // flag to restrict
                                                     // concurrent access
    int * const valuesSolvedPoint; // Flag - is the given point solved
    int * const wereValuesModified; // Flag - was value updated (i.e new value
                                    // calculated and changed by more than the
                                    // precision)
} ThreadArgs;

/**
 * Sets the given square two dimensional array to the following style
 * (of given size):
 *    1 1 1 1 1
 *    1 0 0 0 1
 *    1 0 0 0 1
 *    1 0 0 0 1
 *    1 1 1 1 1
 *
 * That is, edges set to 1, and inner values set to 0. This array represents
 * which values have been solved in function solve, and hence needs to be reset
 * every time one of the values is updated. We can only say we are done when a
 * full pass of the values array in solve has been done without any values
 * being updated.
 *
 * @param solvedArray The two dimensional array to set to the above style
 * @param dimension   The dimension of the two dimensional array given
 */
static void resetSolvedArray(int ** const solvedArray, const int dimension)
{
    for(int row = 0; row < dimension; row++) {
        for(int col = 0; col < dimension; col++) {
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

/**
 * Update row and col to point to the next point that should be calculated.
 *
 * This is complicated as the position of the next point (when moving to the
 * next row) depends on the dimension of the given input array. Below shows
 * an example of both an odd and even dimension grid.
 *
 *   X X X X X          X X X X X X
 *   X E O E X          X E O E O X
 *   X O E O X    AND   X O E O E X
 *   X E O E X          X E O E O X
 *   X X X X X          X O E O E X
 *                      X X X X X X
 *
 * If the current point is second or third last point, we know that we should
 * move to the next row. We always move two spaces at a time (see grids above)
 * and the edges of the grid are fixed. The column we start at on the next row
 * depends on the current column and the dimension of the grid.
 *
 * @param row       The row of the current point, to be updated to row of next
 *                  point. May not change
 * @param col       The column of the current point, to be updated to the
 *                  column of the next point
 * @param dimension The dimension of the two dimensional array we are solving
 */
static void moveToNext(int * const row, int * const col, const int dimension)
{
    if (*col == dimension - 2) {
        // if even dimension, start at first column, otherwise start at second
        *col = isEven(dimension) ? 1 : 2;
        *row += 1;

        return;
    }

    if (*col == dimension - 3) {
        // if even dimension, start at second column, otherwise start at first
        *col = isEven(dimension) ? 2 : 1;
        *row += 1;

        return;
    }

    *col += 2;

    return;
}

/**
 * Check if a point is that last 'even' point to be calculated (see comment at
 * top of this file for definition of what an even/E point is). That is, the
 * bottom right corner of the grid (excluding the fixed edge points).
 *
 * @param  row       The row of the point
 * @param  col       The column of the point
 * @param  dimension The dimension of the two dimensional array we are solving
 *
 * @return           1 if last even point, 0 otherwise
 */
static int isLastEvenPoint(const int row, const int col, const int dimension)
{
    return (row == dimension - 2) && (col == dimension - 2);
}

/**
 * Check if a point is that last 'odd' point to be calculated (see comment at
 * top of this file for definition of what an odd/O point is). That is, one
 * column before the bottom right corner of the grid (excluding the fixed edge
 * points).
 *
 * @param  row       The row of the point
 * @param  col       The column of the point
 * @param  dimension The dimension of the two dimensional array we are solving
 *
 * @return           1 if last odd point, 0 otherwise
 */
static int isLastOddPoint(const int row, const int col, const int dimension)
{
    return (row == dimension - 2) && (col == dimension - 3);
}

/**
 * Runs processes required to end a thread. Updates the flag given to signal
 * that the current thread id has finished and is now (or about to be)
 * available to be used for a new thread. Gains and releases a lock as this
 * flag is updated elsewhere to be 0, so locks must be used for concurrency.
 *
 *
 * @param  threadAvailableFlag     The flag to update
 * @param  threadAvailableFlagLock The lock on the flag to update
 *
 * @return                         NULL
 */
static void *endThread(
    int * const threadAvailableFlag,
    pthread_mutex_t * const threadAvailableFlagLock
)
{
    pthread_mutex_lock(threadAvailableFlagLock);
    *threadAvailableFlag = 1;
    pthread_mutex_unlock(threadAvailableFlagLock);

    return NULL;
}

/**
 * Main callback function for each parallel thread. Updates a specific value
 * to the average of its four neighbours if it changes by more than the given
 * precision. If this happens, it updates a flag (wereValuesModified) to signal
 * that it updated a value. If the value changes by less than the precision, we
 * can (for now) view it as solved, and therefore update the valuesSolvedPoint
 * to 1.
 *
 * @param  values                  The two dimensional array of values being
 *                                 solved
 * @param  row                     The row of the point to update
 * @param  col                     The column of the point to update
 * @param  precision               The precision to compare the change against
 * @param  threadAvailableFlag     The flag to update when thread has finished
 * @param  threadAvailableFlagLock The lock on the flag to update when thread
 *                                 has finished
 * @param  valuesSolvedPoint       The flag to update to signal if the current
 *                                 point is solved
 * @param  wereValuesModified      The flag to update to signal if the value
 *                                 changed by more than the precision and hence
 *                                 was updated
 *
 * @return                         NULL
 */
static void *updateValue(
    double ** const values,
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

    // fabs - absolute value i.e. difference
    if (fabs(newValue - values[row][col]) < precision) {
        *valuesSolvedPoint = 1;

        return endThread(threadAvailableFlag, threadAvailableFlagLock);
    }

    *wereValuesModified = 1;
    values[row][col] = newValue;

    return endThread(threadAvailableFlag, threadAvailableFlagLock);
}

/**
 * Proxy to updateValue for code readbility. The callback to pthread_create can
 * only be passed one parameter (of type void *). This is implemented by
 * defining a struct with the required arguments, and so this simply pulls the
 * required parameters out of this struct and passes it to updateValue.
 * Allows type checking for error checking/debugging and for code readbility.
 *
 * @param  args Struct containing the required arguments for updateValue
 *
 * @return      NULL
 */
static void *updateValueProxy(void *args)
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
    );
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

        /**
         *  Note: Acceptable race condition:
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
            // busy wait, may be CPU hungry but will be faster
            continue;
        }


        rowColSum = row + col;

        // Second condition:
        // reverse version of XOR
        if (!valuesSolvedArray[row][col]) {
            const ThreadArgs args = {
                .values = values,
                .row = row,
                .col = col,
                .precision = precision,
                .threadAvailableFlag = &threadsAvailable[tId],
                .threadAvailableFlagLock = &threadsAvailableLocks[tId],
                .valuesSolvedPoint = &valuesSolvedArray[row][col],
                .wereValuesModified = &wereValuesModified
            };

            pthread_mutex_lock(&threadsAvailableLocks[tId]);

            // create the thread
            pthread_create(&tIds[tId], NULL, updateValueProxy, (void *)&args);
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

        pthread_join(tIds[i], NULL);
    }

    freeTwoDIntArray(valuesSolvedArray, dimension);
}
