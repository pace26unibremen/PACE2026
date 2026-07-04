#include "ClusterBudget.hpp"

namespace solver
{

ClusterBudget::ClusterBudget(clock::time_point start, double totalSeconds, unsigned int numClusters) :
        end_(start + std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(totalSeconds))),
        numClusters_(numClusters == 0 ? 1 : numClusters)
{}

ClusterBudget::clock::time_point ClusterBudget::deadlineFor(unsigned int index, clock::time_point now) const
{
    const clock::duration remaining = end_ - now;
    if (remaining <= clock::duration::zero())
    {
        return now;  // budget already spent -> stop as soon as a solution exists
    }
    const unsigned int remainingClusters = (index >= numClusters_) ? 1 : (numClusters_ - index);
    return now + remaining / static_cast<clock::rep>(remainingClusters);
}

}  // namespace solver
