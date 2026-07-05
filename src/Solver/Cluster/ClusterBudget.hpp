#ifndef PACE2026_CLUSTER_BUDGET_HPP
#define PACE2026_CLUSTER_BUDGET_HPP

#include <chrono>
#include <vector>

namespace solver
{

/// \brief Divides a total wall-clock time budget across a fixed number of clusters,
///        weighted by each cluster's estimated difficulty.
///
/// Clusters are solved one after another in the dependency-preserving order fixed by
/// \ref ClusterSolver::sortClusters; this class does not reorder them. It only decides
/// how much of the remaining wall-clock budget each cluster may spend before the solver
/// should fall back to the best solution found so far and move on to the next cluster.
///
/// Each cluster is granted a share of the budget still remaining at the moment it starts,
/// proportional to its weight relative to the weights of the clusters not yet started. So
/// time left unused by clusters that finish early automatically rolls forward to the
/// clusters that follow, the final cluster may use whatever is left, and larger/harder
/// clusters are budgeted more time than trivial ones instead of an equal 1/N share. Pass
/// uniform weights (or use the \c numClusters constructor) to recover the plain equal split.
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
    /// \param numClusters  number of clusters to divide the budget equally across (treated as >= 1)
    ClusterBudget(clock::time_point start, double totalSeconds, unsigned int numClusters);

    /// \param start        when the cluster-solving phase began
    /// \param totalSeconds total wall-clock budget shared by all clusters
    /// \param weights      per-cluster difficulty weights, in solving order. An empty vector is
    ///                     treated as a single cluster; if the weights of the remaining clusters
    ///                     sum to zero the split falls back to an equal share by count.
    ClusterBudget(clock::time_point start, double totalSeconds, std::vector<double> weights);

    /// \brief Deadline for the cluster at \p index, evaluated at the current time \p now.
    ///
    /// Returns \p now plus the fraction \c weight[index] / (sum of weights from \p index
    /// through the last cluster) of the time remaining until the overall budget end. Never
    /// returns a time before \p now: once the overall budget is spent every remaining
    /// cluster receives an immediate (already-passed) deadline equal to \p now.
    [[nodiscard]] clock::time_point deadlineFor(unsigned int index, clock::time_point now) const;

  private:
    clock::time_point end_;
    std::vector<double> weights_;
};

}  // namespace solver

#endif  // PACE2026_CLUSTER_BUDGET_HPP
