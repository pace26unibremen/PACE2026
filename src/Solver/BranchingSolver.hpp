#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"
#include "Rule/AbstractRule.hpp"
#include  "DebugPlugin.hpp"

#include <stack>

namespace solver
{

class BranchingSolver : public AbstractSolver
{
  protected:
    /// \brief stores all applied rules
    std::stack<std::shared_ptr<AbstractRule>> changes = std::stack<std::shared_ptr<AbstractRule>>();

    /// \brief stores all applied rules that have to be revoked to get the final solution
    std::stack<std::shared_ptr<AbstractRule>> temporalChanges = std::stack<std::shared_ptr<AbstractRule>>();

    /// \brief A debug plugin, nullptr for no additional debug info
    std::shared_ptr<DebugPlugin> debPlugin = nullptr;

  public:
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance);

    ~BranchingSolver() override = default;

    std::shared_ptr<graph::Forest> solve() override;

    std::shared_ptr<DebugPlugin>& DebPlugin();
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
