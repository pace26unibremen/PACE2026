#include "ClusterBudget.hpp"

#include <utility>

namespace solver
{

ClusterBudget::ClusterBudget(clock::time_point start, double totalSeconds, unsigned int numClusters) :
        ClusterBudget(start, totalSeconds, std::vector<double>(numClusters == 0 ? 1 : numClusters, 1.0))
{}

ClusterBudget::ClusterBudget(clock::time_point start, double totalSeconds, std::vector<double> weights) :
        end_(start + std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(totalSeconds))),
        weights_(std::move(weights))
{
    // An empty instance is treated as a single cluster owning the whole budget.
    if (weights_.empty())
    {
        weights_.push_back(1.0);
    }
}

ClusterBudget::clock::time_point ClusterBudget::deadlineFor(unsigned int index, clock::time_point now) const
{
    const clock::duration remaining = end_ - now;
    if (remaining <= clock::duration::zero())
    {
        return now;  // budget already spent -> stop as soon as a solution exists
    }

    // Clusters at or past the end own the whole remaining budget (mirrors the old
    // numClusters - index == 1 case for an out-of-range index).
    if (index >= weights_.size())
    {
        return now + remaining;
    }

    // Weight of this cluster relative to all clusters not yet started (index..end).
    double remainingWeight = 0.0;
    for (unsigned int i = index; i < weights_.size(); ++i)
    {
        remainingWeight += weights_[i];
    }

    const double myWeight = weights_[index];
    // Degenerate weights (all zero from here on): fall back to an equal split by count so a
    // zero-weight tail still divides the remaining time instead of dividing by zero.
    const double fraction = (remainingWeight > 0.0)
                                ? (myWeight / remainingWeight)
                                : (1.0 / static_cast<double>(weights_.size() - index));

    return now + std::chrono::duration_cast<clock::duration>(remaining * fraction);
}

}  // namespace solver
