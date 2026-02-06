#ifndef PACE2026_CONTEXT_HPP
#define PACE2026_CONTEXT_HPP

namespace solver
{

/// \brief A context stores information about the instance and the state of the branching solver.
struct Context
{
    /// \brief The size (number of trees) of the best solution, that the solver found so far.
    /// Default value is max_int.
    unsigned int bestSolutionSize = -1;
};

}  //namespace solver

#endif  //PACE2026_CONTEXT_HPP
