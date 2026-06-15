#include "ClusterRange.hpp"

#include "ClusterSolver.hpp"
#include "../Rule/SingleVertexTreePropagationRule.hpp"

solver::ClusterRange::ClusterRange(solver::ClusterSolver* solver) :
    solver(solver)
{}

void solver::ClusterRange::cutClusterTerminals(unsigned int index)
{
    auto c = solver->cluster.at(index);

    std::unordered_set<unsigned int> cutClusterTerminals;
    for (auto l : solver->clusterToClusterLabels.at(c))
    {
        if (solver->cuttedClusterRoots.contains(l+1))
        {
            cutClusterTerminals.insert(l);
        }
    }
    if (not cutClusterTerminals.empty())
    {
        auto r = solver::SingleVertexTreePropagationRule(c,solver->context,cutClusterTerminals);
        r.apply();
    }
}

void solver::ClusterRange::collectCuttedClusterRoot(unsigned int index)
{
    auto cluster = solver->cluster.at(index);
    auto clusterRootLabel = solver->clusterToRootLabel.at(cluster);
    if (clusterRootLabel != 0)
    {
        auto clusterRootNode = cluster->at(0)->LabelToTerminal().at(clusterRootLabel);
        if (clusterRootNode->isTrueTerminal() and not clusterRootNode->parent)
        {
            solver->cuttedClusterRoots.insert(clusterRootLabel);
        }
    }
}

solver::ClusterRange::ClusterRangeIterator solver::ClusterRange::begin()
{
    return ClusterRangeIterator(0, this);
}

solver::ClusterRange::ClusterRangeIterator solver::ClusterRange::end()
{
    return ClusterRangeIterator(solver->cluster.size(), this);
}

solver::ClusterRange::ClusterRangeIterator::ClusterRangeIterator(unsigned int currentIndex, ClusterRange* clusterRange)
    : currentIndex(currentIndex), clusterRange(clusterRange)
{}

solver::ClusterRange::ClusterRangeIterator::value_type solver::ClusterRange::ClusterRangeIterator::operator*() const
{
    auto context = std::make_shared<Context>();
    auto cluster = clusterRange->solver->cluster.at(currentIndex);

    // add the label of the cluster root the context
    unsigned int clusterRootLabel = clusterRange->solver->clusterToRootLabel.at(cluster);
    if (clusterRootLabel != 0)
    {
        context->clusterRoot.insert(clusterRootLabel); // Todo redundant
        context->clusterRootLabel = clusterRootLabel;
    }

    return {cluster, context};
}

solver::ClusterRange::ClusterRangeIterator& solver::ClusterRange::ClusterRangeIterator::operator++()
{
    // check if the cluster root in the last cluster is cutted
    clusterRange->collectCuttedClusterRoot(currentIndex);
    // step forward
    currentIndex++;
    if (currentIndex == clusterRange->solver->cluster.size())
        return *this;
    // cut the cluster terminals in the current cluster,
    // for which the corresponding cluster root was cutted in the previous clusters
    clusterRange->cutClusterTerminals(currentIndex);
    return *this;
}

solver::ClusterRange::ClusterRangeIterator solver::ClusterRange::ClusterRangeIterator::operator++(int)
{
    ClusterRangeIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool solver::ClusterRange::ClusterRangeIterator::operator==(const ClusterRangeIterator& other) const
{
    return currentIndex == other.currentIndex;
}

bool solver::ClusterRange::ClusterRangeIterator::operator!=(const ClusterRangeIterator& other) const
{
    return not(*this == other);
}
