#ifndef PACE2026_CLUSTER_BUDGET_HPP
#define PACE2026_CLUSTER_BUDGET_HPP

#include <chrono>

namespace solver
{

/// \brief Divides a total wall-clock time budget across a fixed number of clusters.
///
/// Clusters are solved one after another in the dependency-preserving order fixed by
/// \ref ClusterSolver::sortClusters; this class does not reorder them. It only decides
/// how much of the remaining wall-clock budget each cluster may spend before the solver
/// should fall back to the best solution found so far and move on to the next cluster.
///
/// Each cluster is granted an equal share of the budget still remaining at the moment it
/// starts, so time left unused by clusters that finish early automatically rolls forward
/// to the clusters that follow, and the final cluster may use whatever is left.
///
/// \note A per-cluster deadline can only take effect once that cluster has produced at
///       least one feasible solution (see \ref BranchingSolver): until a seed exists the
///       solver keeps searching so that it never emits an empty cluster.
class ClusterBudget
{
  public:
    using clock = std::chrono::steady_clock;

    /// \param start        when the cluster-solving phase began
    /// \param totalSeconds total wall-clock budget shared by all clusters
    /// \param numClusters  number of clusters to divide the budget across (treated as >= 1)
    ClusterBudget(clock::time_point start, double totalSeconds, unsigned int numClusters);

    /// \brief Deadline for the cluster at \p index, evaluated at the current time \p now.
    ///
    /// Returns \p now plus an equal share of the time remaining until the overall budget
    /// end, split across the clusters not yet started (\p index through the last). Never
    /// returns a time before \p now: once the overall budget is spent every remaining
    /// cluster receives an immediate (already-passed) deadline equal to \p now.
    [[nodiscard]] clock::time_point deadlineFor(unsigned int index, clock::time_point now) const;

  private:
    clock::time_point end_;
    unsigned int numClusters_;
};

}  // namespace solver

#endif  // PACE2026_CLUSTER_BUDGET_HPP
