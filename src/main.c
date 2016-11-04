#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug/debug.h"
#include "array/array.h"
#include "problem/problem.h"
#include "solve/solve.h"

/**
 * Checks if any of the parameters passed via CLI are --help or -h
 *
 * @param  args Number of command line argmuments
 * @param  argv Array of command line arguments
 *
 * @return      1 if true (help flag specified), 0 otherwise
 */
const int isHelpFlag(int args, char *argv[])
{
    for (int i = 0; i < args; i++) {
        if (strcmp(argv[i], "--help") == 0
            || strcmp(argv[i], "-h") == 0) {

            return 1;
        }
    }

    return 0;
}

int runSolve(const int problemId, const int threads, const double precision)
{
    const int dimension = getProblemDimension(problemId);

    double ** const values = createTwoDDoubleArray(dimension);
    const int result = fillProblemArray(values, problemId);

    if (result == -1) {
        printf("Invalid problem id given. Must be 1, 2, 3, 4 or 5.\n");

        return -1;
    }

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

int main(int args, char *argv[])
{
    if (isHelpFlag(args, argv)) {
        printf(
            "Argument order:\n"
            " - Problem ID (1, 2, 3, 4 or 5. See src/problem/problem.c).\n"
            " - Number of threads to use.\n"
            " - Precision to work to.\n"
        );

        return 0;
    }

    if (args != 4) {
        printf(
            "You must specify problem ID, "
            "number of threads and precision.\n"
        );

        return -1;
    }

    return runSolve(atoi(argv[1]), atoi(argv[2]), atof(argv[3]));
}
