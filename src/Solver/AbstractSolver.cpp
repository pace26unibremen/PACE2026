#include "AbstractSolver.hpp"

graph::Instance& solver::AbstractSolver::Instance()
{
    return this->instance;
}

const graph::Instance& solver::AbstractSolver::Instance() const
{
    return this->instance;
}