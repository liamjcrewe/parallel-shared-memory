#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "output/output.h"
#include "array/array.h"
#include "problem/problem.h"
#include "solve/solve.h"

#define HELP "Argument order:\n"\
             " - Problem ID (1, 2, 3, 4, 5 or 6. See src/problem/problem.c).\n"\
             " - Number of threads to use.\n"\
             " - Precision to work to.\n"

#define INVALID_NUM_ARGS "You must specify problem ID, "\
                         "number of threads and precision.\n"

#define INVALID_PROBLEM_ID "Invalid problem id given. "\
                           "Must be 1, 2, 3, 4, 5 or 6.\n"

#define INVALID_THREADS "Threads must be an integer greater than 0\n"

#define INVALID_PRECISION "Precision must be a decimal greater than 0\n"

#define PTHREAD_ERROR "Something went wrong. Error code: %d\n"

/**
 * Checks if any of the parameters passed via CLI are --help or -h
 *
 * @param  args Number of command line argmuments
 * @param  argv Array of command line arguments
 *
 * @return      1 if true (help flag specified), 0 otherwise
 */
static const int isHelpFlag(int args, char *argv[])
{
    for (int i = 0; i < args; i++) {
        if (strcmp(argv[i], "--help") == 0
            || strcmp(argv[i], "-h") == 0) {

            return 1;
        }
    }

    return 0;
}

/**
 * Builds array of values based on the problemId given. Checks this is valid,
 * runs solve on these values and writes the solution to file.
 *
 * @param  problemId ID of problem to solve
 * @param  threads   Number of threads to use (upper bound)
 * @param  precision Precision to work solution to
 *
 * @return           0 if success, -1 if error
 */
static int runSolve(
    const int problemId,
    const int threads,
    const double precision
)
{
    const int dimension = getProblemDimension(problemId);

    double ** const values = createTwoDDoubleArray(dimension);
    const int result = fillProblemArray(values, problemId);

    if (result == -1) {
        printf(INVALID_PROBLEM_ID);

        return -1;
    }

    FILE * const f = fopen("./output.txt", "w");

    // Log input
    fprintf(f, "Input:\n");
    write2dDoubleArray(f, values, dimension);

    // Solve and update values
    int error = solve(values, dimension, threads, precision);

    if (error) {
        printf(PTHREAD_ERROR, error);
    }

    // Log solution
    fprintf(f, "Solution:\n");
    write2dDoubleArray(f, values, dimension);

    fclose(f);

    // Free memory
    freeTwoDDoubleArray(values, dimension);

    return error ? error : 0;
}

/**
 * Main function. Runs simple CLI tool that allows --help/-h, and reports an
 * error if not enough/too many command line parameters are passed.
 *
 * @param  args Number of command line arguments
 * @param  argv Array of command line arguments
 * @return      0 if success, -1 if error.
 */
int main(int args, char *argv[])
{
    if (isHelpFlag(args, argv)) {
        printf(HELP);

        return 0;
    }

    if (args != 4) {
        printf(INVALID_NUM_ARGS);

        return -1;
    }

    const int problemId = atoi(argv[1]);
    const int threads = atoi(argv[2]);
    const double precision = atof(argv[3]);

    if (problemId <= 0) {
        printf(INVALID_PROBLEM_ID);

        return -1;
    }

    if (threads <= 0) {
        printf(INVALID_THREADS);

        return -1;
    }

    if (precision <= 0) {
        printf(INVALID_PRECISION);

        return -1;
    }

    return runSolve(problemId, threads, precision);
}
