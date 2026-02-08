#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"
#include "DebugPlugin.hpp"

#include "Rule/Context.hpp"
#include "Rule/AbstractRule.hpp"
#include "Rule/EqualForestsRule.hpp"
#include "Rule/PairEqualRule.hpp"
#include "Rule/PairPathBranchingRule.hpp"
#include "Rule/PairUnconnectedBranchingRule.hpp"
#include "Rule/SingleVertexTreePropagationRule.hpp"
#include "Rule/DebugAssertFalseRule.hpp"

#include <stack>
#include <functional>

namespace solver
{

typedef std::function<std::shared_ptr<AbstractRule>(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context)> isApplicableFn;

class BranchingSolver : public AbstractSolver
{
  protected:
    /// \brief stores all applied rules
    std::stack<std::shared_ptr<AbstractRule>> changes = std::stack<std::shared_ptr<AbstractRule>>();

    /// \brief stores all applied rules that have to be revoked to get the final solution
    std::stack<std::shared_ptr<AbstractRule>> temporalChanges = std::stack<std::shared_ptr<AbstractRule>>();

    /// \brief Stores the best solution, that the solver found so far.
    std::shared_ptr<graph::Forest> solution = nullptr;

    /// \brief A debug plugin, nullptr for no additional debug info
    std::shared_ptr<DebugPlugin> debPlugin = nullptr;

    /// \brief Context information about the instance and the solver state
    std::shared_ptr<Context> context = std::make_shared<Context>();

    /// \brief unapply all rules until the next branching possibility
    /// \returns Whether all rules are unapplied. (Means there is no branching possibility left)
    bool rollBackBranch();

    /// \brief Checks whether the current instance is a better solution than the current candidate.
    /// If true, it restores all temporal changes on the instance and writes a copy in the solution field.
    /// \warning Assumes that the current instance is a solution.
    void checkSolutionCandidate();

    /// \brief vector of the isApplicable function of rules.
    /// It defines which rules are used and in which order they are checked for applicability.
    std::vector<isApplicableFn> activeRules = {
        solver::EqualForestsRule::isApplicable,
        solver::SingleVertexTreePropagationRule::isApplicable,
        solver::PairUnconnectedBranchingRule::isApplicable,
        solver::PairEqualRule::isApplicable,
        solver::PairPathBranchingRule::isApplicable,
        solver::DebugAssertFalseRule::isApplicable
        };

  public:
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance);

    ~BranchingSolver() override = default;

    std::shared_ptr<graph::Forest> solve() override;

    /// \brief setter / getter for the debPlugin field
    /// \see BranchingSolver::debPlugin
    std::shared_ptr<DebugPlugin>& DebPlugin();

    /// \brief setter / getter for the activeRules field
    /// \see BranchingSolver::activeRules
    std::vector<isApplicableFn>& ActiveRules();
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
