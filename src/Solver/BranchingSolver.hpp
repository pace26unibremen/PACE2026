#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"
#include "DebugPlugin.hpp"
#include "Context.hpp"

#include "Rule/Context.hpp"
#include "Rule/AbstractRule.hpp"
#include "Rule/CutBranchRule.hpp"
#include "Rule/EqualForestsRule.hpp"
#include "Rule/PairEqualRule.hpp"
#include "Rule/PairPathBranchingRule.hpp"
#include "Rule/PairUnconnectedBranchingRule.hpp"
#include "Rule/SingleVertexTreePropagationRule.hpp"
#include "Rule/DebugAssertFalseRule.hpp"

#include <functional>
#include <stack>
#include <queue>

namespace solver
{

/// \brief a function type that maps an instance and a context to a rule
using isApplicableFn = std::function<std::shared_ptr<AbstractRule>(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context)>;


/// \brief The Branching Solver solves the MAF problem by repeatedly applying \ref AbstractRule "rules",
/// including \ref AbstractBranchingRule "branching rules", so that the solver's search space is a tree.
class BranchingSolver : public AbstractSolver
{
  protected:
    /// \brief stores all applied rules of the current branch in the order in which they were applied.
    std::list<std::shared_ptr<AbstractRule>> appliedRules = std::list<std::shared_ptr<AbstractRule>>();

    /// \brief queue of rules, that should be applied next
    std::queue<std::shared_ptr<AbstractRule>> applyNext = std::queue<std::shared_ptr<AbstractRule>>();

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
    /// \note Assumes that the current instance is a solution.
    void checkSolutionCandidate();

    /// \brief vector of the isApplicable function of rules.
    /// It defines which rules are used and in which order they are checked for applicability.
    std::vector<isApplicableFn> activeRules = {
        solver::CutBranchRule::isApplicable,
        solver::EqualForestsRule::isApplicable,
        solver::SingleVertexTreePropagationRule::isApplicable,
        solver::PairUnconnectedBranchingRule::isApplicable,
        solver::PairEqualRule::isApplicable,
        solver::PairPathBranchingRule::isApplicable,
        solver::DebugAssertFalseRule::isApplicable
        };

  public:
    /// \brief Constructor for a branching solver.
    /// \param instance to solve
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance);

    ~BranchingSolver() override = default;

    /// \brief starts the solver
    /// \returns the solution
    std::shared_ptr<graph::Forest> solve() override;

    /// \brief setter / getter for the debug plugin field
    /// \property DebPlugin
    /// \ref BranchingSolver::debPlugin
    std::shared_ptr<DebugPlugin>& DebPlugin();

    /// \brief setter / getter for the active rules field
    /// \property ActiveRules
    /// \ref BranchingSolver::activeRules
    std::vector<isApplicableFn>& ActiveRules();
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
