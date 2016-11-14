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
int solve(
    double ** const values,
    const int dimension,
    const int threads,
    const double precision
);
