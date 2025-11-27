#include "BranchingSolver.hpp"

solver::BranchingSolver::BranchingSolver(graph::Instance& instance)
{
    this->instance = std::move(instance);
}

graph::Forest solver::BranchingSolver::solve()
{
    return this->instance[0];
}
