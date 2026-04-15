#ifndef PACE2026_CONTEXT_HPP
#define PACE2026_CONTEXT_HPP

#include <unordered_set>

#include "../../Graph/Node.hpp"

namespace solver
{

/// \brief A context stores information about the instance and the state of the branching solver.
struct Context
{
    /// \brief The size (number of trees) of the best solution, that the solver found so far.
    /// Default value is max_int.
    unsigned int bestSolutionSize = -1;

    /// \brief A set of all protected edges (edges that must never be cut).
    /// An edge is identified by the node it points to (the child-node).
    std::unordered_set<graph::Node*> protectedEdges = std::unordered_set<graph::Node*>();
};

}  //namespace solver

#endif  //PACE2026_CONTEXT_HPP
