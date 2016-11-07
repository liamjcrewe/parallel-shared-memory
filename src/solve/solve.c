/**
 * Algorithm background:
 * ---------------------
 *
 * This file contains most of the complicated logic to solve the problem using
 * a parallel algorithm. It does, in places, sacrifice code readability for
 * speed and efficiency, hence the long comments. To give some background to
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
 *       O = 'odd' point - where row and column indices add to an odd value,
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
    double ** values; // The two dimensional array of values being solved
    int row; // The row to update in the thread
    int col; // The column to update in the thread
    double precision; // The precision to work to
    int * threadAvailableFlag; // Flag - is a given thread available
    pthread_mutex_t * threadAvailableFlagLock; // Lock on availability flag to
                                               // restrict concurrent access
    int ** valuesSolvedArray; // Two dimensional array of flags that signal
                              // which values have been solved
    int valuesSolvedArrayDimension; // The dimension of valuesSolvedArray
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
 * @param valuesSolvedArray The two dimensional array to set to the above style
 * @param dimension   The dimension of the two dimensional array given
 */
static void resetSolvedArray(int ** const valuesSolvedArray, const int dimension)
{
    for(int row = 0; row < dimension; row++) {
        for(int col = 0; col < dimension; col++) {
            if (row == 0 || row == dimension - 1
                || col == 0 || col == dimension - 1
            ) {
                valuesSolvedArray[row][col] = 1;

                continue;
            }
            valuesSolvedArray[row][col] = 0;
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
 * Move to the next 'pass' of the values array. Reset the indices to the first
 * point to calculate, and update the oddPointsFlag to signal that we are doing
 * different points in the next pass.
 *
 * @param oddPointsFlag The flag signalling if we are doing 'odd' points
 * @param row           The row index to reset
 * @param col           The column index to reset
 */
static void moveToNextPass(
    int * const oddPointsFlag,
    int * const row,
    int * const col
)
{
    // Do different points next time
    *oddPointsFlag = *oddPointsFlag ? 0 : 1;

    // Reset indices for next 'pass'
    *row = 1;
    *col = *oddPointsFlag ? 2 : 1;
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
 * Main callback function for each parallel thread. Updates a specific value to
 * the average of its four neighbours if it changes by more than the given
 * precision. If this happens, it resets the valuesSolvedArray, as we cannot
 * assume any pre calculated values are 'solved' if a value is updated. If the
 * value changes by less than the precision, we can (for now) view it as
 * solved, and therefore update the corresponding point in the
 * valuesSolvedArray to 1.
 *
 * @param  values                     The two dimensional array of values being
 *                                    solved
 * @param  row                        The row of the point to update
 * @param  col                        The column of the point to update
 * @param  precision                  The precision to compare the change
 *                                    against
 * @param  threadAvailableFlag        The flag to update when thread has
 *                                    finished
 * @param  threadAvailableFlagLock    The lock on the flag to update when
 *                                    thread has finished
 * @param  valuesSolvedArray          Two dimensional array of flags that
 *                                    signal which values have been solved
 * @param  valuesSolvedArrayDimension The dimension of valuesSolvedArray
 *
 * @return                            NULL
 */
static void *updateValue(
    double ** const values,
    const int row,
    const int col,
    const double precision,
    int * const threadAvailableFlag,
    pthread_mutex_t * const threadAvailableFlagLock,
    int ** const valuesSolvedArray,
    const int valuesSolvedArrayDimension
)
{
    double newValue = (values[row][col - 1] + values[row][col + 1]
                        + values[row - 1][col] + values[row + 1][col]) / 4;

    // fabs - absolute value i.e. difference
    if (fabs(newValue - values[row][col]) < precision) {
        valuesSolvedArray[row][col] = 1;

        return endThread(threadAvailableFlag, threadAvailableFlagLock);
    }

    values[row][col] = newValue;

    // We've changed a value so must assume all values are unsolved and recheck
    resetSolvedArray(valuesSolvedArray, valuesSolvedArrayDimension);

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
    const ThreadArgs * const threadArgs = (ThreadArgs*) args;

    void *res = updateValue(
        threadArgs->values,
        threadArgs->row,
        threadArgs->col,
        threadArgs->precision,
        threadArgs->threadAvailableFlag,
        threadArgs->threadAvailableFlagLock,
        threadArgs->valuesSolvedArray,
        threadArgs->valuesSolvedArrayDimension
    );

    free(args);

    return res;
}

/**
 * Check if the solution is 'solved'. That is, the valuesSolved array does not
 * contain a '0' flag (which represents an unsolved value)
 *
 * @param  valuesSolvedArray Two dimensional array of flags that signal which
 *                           values have been solved
 * @param  dimension         The dimension of valuesSolvedArray
 *
 * @return                   1 if valuesSolvedArray does not contain 0 (i.e
 *                           solution is 'solved'), 0 otherwise
 */
static int isSolved(int ** const valuesSolvedArray, const int dimension)
{
    return !twoDIntArrayContains(0, valuesSolvedArray, dimension);
}

/**
 * Check if all threads have finished. If the threadsAvailable array contains a
 * 0, then we know a thread is still running (as the last thing a thread does
 * is update it's corresponding entry in this array to 1).
 * All threads have finished if the given threadsAvailable array does not
 * contain a 0.
 *
 * @param  threadsAvailable Array of flags singalling which threads are
 *                          available
 * @param  threads          Max number of threads, which is the dimension of
 *                          the threadsAvailable array
 *
 * @return                  1 if threadsAvailable does not contain 0 (i.e all
 *                          threads have finished), 0 otherwise
 */
static int allThreadsFinished(int * const threadsAvailable, const int threads)
{
    // intArraySearch returns -1 if it doesn't find given value
    return intArraySearch(0, threadsAvailable, threads) == -1;
}

/**
 * Solve the given values array and update it to the solution. Replaces each
 * point with the average of its four neighbours and repeats until the point
 * changes by less than the given precision. Does this until all points satisfy
 * this criteria. Uses a given number of threads to solve the problem in
 * parallel.
 *
 * @param values    The two dimensional values array to solve and update to the
 *                  solution
 * @param dimension The dimension of the two dimensional values array
 * @param threads   The number of threads to use when solving the problem (note
 *                  this is an upper bound)
 * @param precision The precision to work to (stop updating values when they
 *                  change by less than the precision)
 */
void solve(
    double ** const values,
    const int dimension,
    const int threads,
    const double precision
)
{
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

    // start at (1, 1) as edges are fixed.
    int row = 1, col = 1;

    // start with 'E' points
    int oddPointsFlag = 0;

    // Create array to flag which values have been solved ...
    int ** const valuesSolvedArray = createTwoDIntArray(dimension);
    // and initially set it
    resetSolvedArray(valuesSolvedArray, dimension);

    /**
     *  Only terminate solution is solved and all threads have finished
     *  Note it is important that these checks are in this order, as swapping
     *  the order could lead to a race where the isSolved check passes, then a
     *  thread resets the valuesSolved array and terminates, then the second
     *  check passes even though we do not have the complete solution.
     */
    while (!(allThreadsFinished(threadsAvailable, threads)
           && isSolved(valuesSolvedArray, dimension))
    ) {
        /**
         * Find index (ID) of next available thread to spawn
         *  Note: Acceptable race condition here:
         *  -------------------------------------
         *      If a thread is updating it's own ID in availableThreads to
         *      1 ('available'), this will miss this as it does not acquire the
         *      lock. However, this is a design choice. This will simply loop
         *      and try again rather than have the overhead of acquiring the
         *      lock.
         */
        tId = intArraySearch(1, threadsAvailable, threads);

        // If no threads available
        if (tId == -1) {
            // busy wait, may be CPU hungry but will be faster
            continue;
        }

        // If value isn't already solved, spawn thread to update it
        if (!valuesSolvedArray[row][col]) {
            // Dynamically allocated as we need a fresh instance every time
            // This is freed in the callback (updateValueProxy)
            ThreadArgs * const args = malloc(sizeof(ThreadArgs));
            args->values = values;
            args->row = row;
            args->col = col;
            args->precision = precision;
            args->threadAvailableFlag = &threadsAvailable[tId];
            args->threadAvailableFlagLock = &threadsAvailableLocks[tId];
            args->valuesSolvedArray = valuesSolvedArray;
            args->valuesSolvedArrayDimension = dimension;

            pthread_mutex_lock(&threadsAvailableLocks[tId]);

            // create the thread
            pthread_create(&tIds[tId], NULL, updateValueProxy, (void *)args);
            // set lock to unavailable
            threadsAvailable[tId] = 0;

            pthread_mutex_unlock(&threadsAvailableLocks[tId]);
        }

        // If we are not at the last point in this 'pass'
        if (!isLastEvenPoint(row, col, dimension)
            && !isLastOddPoint(row, col, dimension)
        ) {
            moveToNext(&row, &col, dimension);

            continue;
        }

        // Here we must be at the last point, so move to next pass
        moveToNextPass(&oddPointsFlag, &row, &col);

        // Wait for all live threads to finish before the next pass, as we
        // cannot do some 'E's and 'O's at the same time (see top of file
        // comment for info on what an 'E' and an 'O' is)
        for (int i = 0; i < threads; i++) {
            // If thread is available, it's finished, so no need to wait
            if (threadsAvailable[i]) {
                continue;
            }

            pthread_join(tIds[i], NULL);
        }
    }

    freeTwoDIntArray(valuesSolvedArray, dimension);
}
