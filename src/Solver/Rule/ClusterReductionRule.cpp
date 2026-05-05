#include "ClusterReductionRule.hpp"

#include "../../Cluster/ClusterPointGenerator.hpp"

#include <algorithm>
#include <ranges>

solver::ClusterReductionRule::ClusterReductionRule(
    const std::shared_ptr<graph::Instance>& instance, const std::shared_ptr<Context>& context,
    const std::list<std::list<std::pair<std::shared_ptr<graph::Forest>, graph::Node*>>>& pointsAndForests_PerCluster)
        : AbstractRule(instance, context, true), pointsAndForests_PerCluster(pointsAndForests_PerCluster)
{}

solver::RuleReturnCode solver::ClusterReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ClusterReductionRule : apply : rule is already applied");
    }
    isApplied = true;

    auto maxLabel = std::ranges::max(instance->at(0)->LabelToTerminal() | std::views::keys);

    for (const auto& _cluster : pointsAndForests_PerCluster)
    {
        for (const auto& [f,n] : _cluster)
        {
            changes.emplace(f,n,maxLabel+1,maxLabel+2);
            changes.top().doAction();
        }

        // TODO should be removed in the unapply
        context->clusterLabel.insert(maxLabel+1);

        maxLabel += 2;
    }

    return RuleReturnCode::Continue;
}

void solver::ClusterReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ClusterReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}

std::shared_ptr<solver::AbstractRule>
solver::ClusterReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                           const std::shared_ptr<Context>& context)
{
    auto cpg = cluster::ClusterPointGenerator::wrappedConstructor(instance);

    auto PointsAndForests_PerCluster = std::list<std::list<std::pair<std::shared_ptr<graph::Forest>, graph::Node*>>>();

    for (auto clusterPointOfFrontForest : cpg.clusterPoints)
    {

        PointsAndForests_PerCluster.push_back({{instance->at(0),clusterPointOfFrontForest}});

        // check to which forest the twin belongs
        for (auto twin : cpg.twinRelation.nodeToTwins[clusterPointOfFrontForest])
        {
            for (const auto& f : *instance)
            {
                const graph::Node* begin = f->Nodes().data();
                const graph::Node* end   = begin + f->Nodes().size();
                if ( twin >= begin && twin < end)
                {
                    PointsAndForests_PerCluster.back().emplace_back(f,twin);
                    break;
                }
            }
        }
    }
    if (not PointsAndForests_PerCluster.empty())
    {
        return std::make_shared<ClusterReductionRule>(instance, context, PointsAndForests_PerCluster);
    }
    return nullptr;
}

std::string solver::ClusterReductionRule::name() const
{
    return "ClusterReductionRule";
}

std::shared_ptr<solver::AbstractRule> solver::ClusterReductionRule::clone() const
{
    return std::make_shared<ClusterReductionRule>(instance, context, pointsAndForests_PerCluster);
}