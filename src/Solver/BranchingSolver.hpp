#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"

namespace solver
{

class BranchingSolver : AbstractSolver
{
  public:
    explicit BranchingSolver(graph::Instance& instance);

    graph::Forest solve() override;
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
