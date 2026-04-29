#ifndef PACE2026_CONTEXT_HPP
#define PACE2026_CONTEXT_HPP

#include <memory>
#include <unordered_set>

#include "../Graph/Node.hpp"

namespace solver
{
// forward declaration of \ref BranchingSolverConfiguration
struct SolverConfiguration;

/// \brief A context stores information about the instance and the state of the branching solver.
struct Context
{
    /// \brief The size (number of trees) of the best solution, that the solver found so far.
    /// Default value is max_int.
    unsigned int bestSolutionSize = -1;

    /// \brief The maximum size (number of trees) of the best solution, that the solver
    /// searches in a bounded depth search. \ref BranchingSolverConfiguration::boundedDephtSearch
    /// Default value is 1.
    unsigned int maxSolutionSize = 1;

    /// \brief The configuration of the branching solver.
    /// This should be set in the constructor of the branching solver.
    std::shared_ptr<SolverConfiguration> solverConfiguration = nullptr;

    /// \brief A set of all protected edges (edges that must never be cut).
    /// An edge is identified by the node it points to (the child-node).
    std::unordered_set<graph::Node*> protectedEdges = std::unordered_set<graph::Node*>();

    /// \brief A set of newly created labels for clusters.
    std::unordered_set<unsigned int> clusterLabel = std::unordered_set<unsigned int>();
};

}  //namespace solver

#endif  //PACE2026_CONTEXT_HPP
