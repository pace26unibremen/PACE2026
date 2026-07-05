#ifndef PACE2026_BRANCHING_SOLVER_HPP
#define PACE2026_BRANCHING_SOLVER_HPP

#include "AbstractSolver.hpp"
#include "BranchingSolverConfiguration.hpp"
#include "Context.hpp"

#include <atomic>
#include <chrono>
#include <queue>

namespace solver
{

/// \brief The Branching Solver solves the MAF problem by repeatedly applying \ref AbstractRule "rules",
/// including \ref AbstractBranchingRule "branching rules", so that the solver's search space is a tree.
class BranchingSolver : public AbstractSolver
{
  protected:
    /// \brief The configuration of the branching solver.
    const std::shared_ptr<BranchingSolverConfiguration> configuration =
        std::make_shared<BranchingSolverConfiguration>();

    /// \brief stores all applied rules of the current branch in the order in which they were applied.
    std::list<std::shared_ptr<AbstractRule>> appliedRules = std::list<std::shared_ptr<AbstractRule>>();

    /// \brief queue of rules, that should be applied next
    std::queue<std::shared_ptr<AbstractRule>> applyNext = std::queue<std::shared_ptr<AbstractRule>>();

    /// \brief Context information about the instance and the solver state
    std::shared_ptr<Context> context = std::make_shared<Context>();

    /// \brief Timeout flag set by an external signal handler. When true, solve() stops at the
    /// next branch rollback and returns whatever solution has been found so far. Null = disabled.
    std::atomic<bool>* timeoutFlag = nullptr;

    /// \brief Soft wall-clock deadline for this solver. Once reached, solve() stops at the next
    /// branch rollback and returns the best solution found so far — exactly like \ref timeoutFlag,
    /// but self-contained (no external signal). Used to time-slice a per-cluster budget so one
    /// hard cluster cannot consume the whole run. Default \c max() = disabled (never expires).
    std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();

    /// \brief How often \ref timeExpired actually reads the clock: once every this many calls.
    /// steady_clock::now() is ~2-4% of the branch loop and is hit on every iteration on the deadline
    /// path, so we amortise it. 1024 is chosen for the deadline-overshoot bound, not the overhead
    /// (already negligible at any stride >= ~256): at ~1 us per iteration (measured, 800 leaves) the
    /// deadline overshoots by at most ~1024 * 1 us ~= 1 ms, nothing against a seconds-scale budget.
    /// A larger stride would only grow that overshoot for no further overhead saving.
    static constexpr unsigned int kClockCheckStride = 1024;

    /// \brief Countdown to the next real clock read in \ref timeExpired (see \ref kClockCheckStride).
    mutable unsigned int clockCheckCountdown = 0;

    /// \brief Cached "deadline has passed" verdict, reused between the throttled clock reads.
    mutable bool deadlinePassed = false;

    /// \brief True once either the external timeout flag is set or \ref deadline has passed.
    /// \note Like the flag, this only stops the search once a solution candidate exists; the
    ///       caller-side guard \c not solutionBranch.empty() is applied at each check site. The
    ///       SIGTERM flag is honoured on every call; only the wall-clock deadline is throttled.
    [[nodiscard]] bool timeExpired() const;

    /// \brief The branch that leads to the solution
    std::list<std::shared_ptr<AbstractRule>> solutionBranch = std::list<std::shared_ptr<AbstractRule>>();

    /// \brief unapply all rules until the next branching possibility
    /// \returns Whether all rules are unapplied. (Means there is no branching possibility left)
    bool rollBackBranch();

    /// \brief Unconditionally unapplies every rule in \ref appliedRules, restoring the instance
    /// to the state it had before the current (abandoned) branch started.
    /// \note Used on timeout: \ref solutionBranch is replayed against the instance afterwards,
    /// so the instance must be back to its pristine state first, or the replay corrupts the tree.
    void unwindAppliedRules();

    /// \brief Checks whether the current instance is a better solution than the current candidate.
    /// If true, it restores all temporal changes on the instance and writes a copy in the solution field.
    /// \note Assumes that the current instance is a solution.
    void checkSolutionCandidate();

    /// \brief initializes solver specific fields in the \ref Context object of the solver.
    void initializeContext();

  public:
    /// \brief Constructor for a branching solver.
    /// \param instance to solve
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance);

    /// \brief Constructor for a branching solver.
    /// \param instance to solve
    /// \param configuration for the branching solver
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance,
                             const std::shared_ptr<solver::BranchingSolverConfiguration>& configuration);

    /// \brief Constructor for a branching solver.
    /// \param instance to solve
    /// \param configuration for the branching solver
    /// \param context additional context for the instance
    explicit BranchingSolver(const std::shared_ptr<graph::Instance>& instance,
                             const std::shared_ptr<solver::BranchingSolverConfiguration>& configuration,
                             const std::shared_ptr<solver::Context>& context);

    ~BranchingSolver() override = default;

    /// \brief starts the solver
    /// \returns true if the solver solves the instance, else false
    bool solve() override;

    /// \brief Unapplies all reduction rules, that where applied to the instance.
    void unapplyReductions() override;

    /// \brief Getter for the Context.
    const std::shared_ptr<solver::Context>& GetContext();

    /// \brief The rules (clones) that make up the best solution found, in application order.
    /// Empty if no solution was found. Valid after \ref solve() returns.
    [[nodiscard]]
    const std::list<std::shared_ptr<AbstractRule>>& SolutionBranch() const
    {
        return solutionBranch;
    }

    /// \brief Pre-seed the search with a known solution (e.g. from the approximation): sets the
    /// incumbent weight so \ref solver::CutBranchRule prunes from the first node, and stores \p branch
    /// as the current best solution. If the search never beats it, \ref solve() re-applies \p branch at
    /// the end and returns it — which also means the solver can stop on SIGTERM even before its own
    /// first leaf, because it already has a solution to emit. The rules in \p branch must be valid,
    /// unapplied rules for this solver's instance (rolled back off it via \ref unapplySolutionBranch).
    void seedSolution(std::list<std::shared_ptr<AbstractRule>> branch, float weight);

    /// \brief Unapply the stored solution branch in reverse application order and clear it, restoring
    /// the instance to the state it had before that branch was applied. Used to roll a finished
    /// approximation run off the (shared) instance while keeping its branch for reuse.
    void unapplySolutionBranch();

    /// \brief Registers a timeout flag. When the flag is set to true (e.g. from a signal handler),
    /// solve() stops at the next branch rollback and returns the best solution found so far.
    /// Pass nullptr to disable. Returns false if no solution existed when the flag fired.
    void setTimeoutFlag(std::atomic<bool>* flag);

    /// \brief Sets a soft wall-clock deadline. Once \p deadline has passed, solve() stops at the
    /// next branch rollback and returns the best solution found so far (never before a first
    /// candidate exists). Pass \c steady_clock::time_point::max() to disable.
    void setDeadline(std::chrono::steady_clock::time_point deadline);
};

}  //namespace solver

#endif  //PACE2026_BRANCHING_SOLVER_HPP
