#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"
#include "Context.hpp"
#include "SolverConfiguration.hpp"

#include <queue>

namespace solver
{

/// \brief The Branching Solver solves the MAF problem by repeatedly applying \ref AbstractRule "rules",
/// including \ref AbstractBranchingRule "branching rules", so that the solver's search space is a tree.
class BranchingSolver : public AbstractSolver
{
  protected:
    /// \brief The configuration of the branching solver.
    const std::shared_ptr<SolverConfiguration>
    configuration = std::make_shared<SolverConfiguration>();

    /// \brief stores all applied rules of the current branch in the order in which they were applied.
    std::list<std::shared_ptr<AbstractRule>> appliedRules = std::list<std::shared_ptr<AbstractRule>>();

    /// \brief queue of rules, that should be applied next
    std::queue<std::shared_ptr<AbstractRule>> applyNext = std::queue<std::shared_ptr<AbstractRule>>();

    /// \brief Context information about the instance and the solver state
    std::shared_ptr<Context> context = std::make_shared<Context>();

    /// \brief The branch that leads to the solution
    std::list<std::shared_ptr<AbstractRule>> solutionBranch =
        std::list<std::shared_ptr<AbstractRule>>();

    /// \brief unapply all rules until the next branching possibility
    /// \returns Whether all rules are unapplied. (Means there is no branching possibility left)
    bool rollBackBranch();

    /// \brief Checks whether the current instance is a better solution than the current candidate.
    /// If true, it restores all temporal changes on the instance and writes a copy in the solution field.
    /// \note Assumes that the current instance is a solution.
    void checkSolutionCandidate();

  public:
    /// \brief Constructor for a branching solver.
    /// \param instance to solve
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance);

    /// \brief Constructor for a branching solver.
    /// \param instance to solve
    /// \param configuration for the branching solver
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance,
                             const std::shared_ptr<solver::SolverConfiguration>& configuration);

    ~BranchingSolver() override = default;

    /// \brief starts the solver
    /// \returns true if the solver solves the instance, else false
    bool solve() override;

    /// \brief Unapplies all reduction rules, that where applied to the instance.
    void unapplyReductions() override;

    /// \brief Getter for the Context.
    const std::shared_ptr<solver::Context>& GetContext();
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
