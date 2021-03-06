/**
 * @file    multiple_board/main/consensus.hpp
 *
 * @brief   Consensus algorithm headers
 *
 * Consensus distributed algorithm implementation for obtaining a solution
 * to a global optimisation problem.
 *
 * @author  João Borrego
 */

#ifndef INO_CONSENSUS_HPP
#define INO_CONSENSUS_HPP

#include "matrix.hpp"
#include "constants.hpp"
#include "communication.hpp"

/** Debug flags for printing intermediate results to Serial */
#define DEBUG_CONSENSUS false

namespace Consensus{

    /** Total number of iterations */
    const float ITERATIONS = 25;
    /** Augmented Lagrangian method penalty parameter */
    const float RHO = 0.01;
    /** Quadratic terms coefficients */
    const float Q[N] = {0.0, 0.0};
    /** Linear terms coefficients */
    const float C[N] = {1.0, 1.0};

    /* Functions */

    /**
     * @brief      Evaluates the cost function.
     *
     * @param      d_i      The solution
     * @param      d_i_avg  The average of nodes' solutions
     * @param[in]  q_i      The q_i non-zero element of Q
     * @param[in]  c_i      The c_i non-zero element of C
     * @param      y_i      The Lagrangian multipliers
     * @param[in]  rho      The penalty parameter
     *
     * @return     The cost
     */
    float costFunction(
        float *d_i,
        float *d_i_avg,
        float q_i,
        float c_i,
        float *y_i, 
        float rho);

    /**
     * @brief      Checks if a solution is feasible.
     *
     * @param      d_i           The solution
     * @param      K_i           The K_i submatrix of K
     * @param[in]  L             The illuminance lower bound
     * @param[in]  o             The external illuminance
     * @param[in]  check_linear  Check linear constraint on minimum illuminance
     *
     * @return     Whether solution is feasible.
     */
    bool checkConstraints(
        float *d_i,
        float *K_i,
        float L,
        float o,
        bool check_linear);

    /**
     * @brief      Tries to update the best solution.
     *
     * @param      d_i           The solution
     * @param      K_i           The K_i submatrix of K
     * @param[in]  L             The illuminance lower bound
     * @param[in]  o             The external illuminance
     * @param[in]  check_linear  Check linear constraint on minimum illuminance
     * @param      d_i_best      The best solution
     * @param      cost_best     The cost of th best solution
     * @param      d_i_avg       The average of nodes' solutions
     * @param[in]  q_i           The q_i non-zero element of Q
     * @param[in]  c_i           The c_i non-zero element of C
     * @param      y_i           The Lagrangian multipliers
     * @param[in]  rho           The penalty parameter
     */
    void updateBest(
        float *d_i,
        float *K_i,
        float L,
        float o,
        bool check_linear,
        float *d_i_best,
        float *cost_best,
        float *d_i_avg,
        float q_i,
        float c_i,
        float *y_i,
        float rho);

    /**
     * @brief      Gets the average solution from neighbours.
     *
     * @param      d_i_best  The local best solution
     */
    void getAverageSolution(float *d_i_best);

    /**
     * @brief      Callback function for receiving I2C messages.
     *
     * @param[in]  bytes  The number of bytes available
     */
    void onReceive(int bytes);

    /**
     * @brief      Executes consensus algorithm.
     *
     * @param[in]  id    The node identifier
     * @param[in]  L     Target lower bound illuminance
     * @param      K_i   The k i submatrix of K
     * @param[in]  o     External illuminance
     * @param[in]  onComplete  On iteration complete custom function pointer
     *
     * @return     Output reference in lux units
     */
    float solve(size_t id, float L, float* K_i, float o, void (*onComplete)(void));

    /**
     * @brief      Prints debug information to Serial.
     *
     * @param      d            The variable array
     * @param[in]  size         The size of the array
     * @param[in]  it           The current iteration of consensus
     * @param      description  The description of the variable array
     */
    void debugPrint(float *d, int size, int it, char* description);

    /**
     * @brief      Ensure every node has every other nodes' solutions.
     *
     * @param      d_i_best  The current node's solution
     */
    void getCopies(float *d_i_best);
}

#endif
