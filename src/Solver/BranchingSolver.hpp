#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"
#include "Rule/AbstractRule.hpp"

#include <stack>

namespace solver
{

class BranchingSolver : AbstractSolver
{
  protected:
    std::stack<std::shared_ptr<AbstractRule>> changes = std::stack<std::shared_ptr<AbstractRule>>();

  public:
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance);

    ~BranchingSolver() override = default;

    std::shared_ptr<graph::Forest> solve() override;
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
