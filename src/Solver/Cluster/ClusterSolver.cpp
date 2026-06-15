#include "ClusterSolver.hpp"
#include "ClusterRange.hpp"

std::shared_ptr<graph::Instance> solver::ClusterSolver::buildSingleCluster(unsigned int i)
{
    auto subInstance = std::make_shared<graph::Instance>();
    for (const auto& f : *instance)
    {
        graph::Node* root = f->Roots()[i];
        std::unordered_map<graph::Node*, unsigned int> mapTL = f->TerminalToLabel();
        std::unordered_map<unsigned int, graph::Node*> mapLT = f->LabelToTerminal();
        std::vector<graph::Node*> roots;

        roots.push_back(root);
        std::erase_if(mapTL, [root](const auto& entry) { return not root->hasTerminal(entry.second); });
        std::erase_if(mapLT, [root](const auto& entry) { return not root->hasTerminal(entry.first); });

        auto forest =
            std::make_shared<graph::Forest>(std::make_shared<std::vector<graph::Node>>(f->Nodes()),
                                            std::make_shared<std::unordered_map<unsigned int, graph::Node*>>(mapLT),
                                            std::make_shared<std::unordered_map<graph::Node*, unsigned int>>(mapTL),
                                            std::make_shared<std::vector<graph::Node*>>(roots));
        subInstance->push_back(forest);
    }
    return subInstance;
}

void solver::ClusterSolver::mergeCluster()
{
    // we only need the first forest
    instance->erase(instance->begin() + 1, instance->end());
    instance->at(0)->Roots().clear();
    for (auto & c : cluster)
    {
        instance->at(0)->Roots().insert(instance->at(0)->Roots().end(), c->at(0)->Roots().begin(),
                                        c->at(0)->Roots().end());
    }
    std::sort(instance->at(0)->Roots().begin(), instance->at(0)->Roots().end(),
              [](graph::Node* r1, graph::Node* r2) { return r1->hasSmallestTerminal(r2); });
}

void solver::ClusterSolver::splitInstance()
{
    for (unsigned int i = 0; i < instance->at(0)->Roots().size(); ++i)
    {
        auto c = buildSingleCluster(i);
        cluster.push_back(c);

        // fill maps rootLabelToCluster, clusterToRootLabel, clusterToClusterLabels
        bool isRootCluster = true;
        for (unsigned int l : context->clusterRoot)
        {
            if (c->at(0)->LabelToTerminal().contains(l))
            {
                clusterToRootLabel.insert({c,l});
                rootLabelToCluster.insert({l,c});
                isRootCluster = false;
                break;
            }
        }
        if(isRootCluster)
        {
            clusterToRootLabel.insert({c,0});
            rootLabelToCluster.insert({0,c});
        }
        clusterToClusterLabels.insert({c,{}});
        for (unsigned int l : context->clusterLabel)
        {
            if (c->at(0)->LabelToTerminal().contains(l))
            {
                clusterToClusterLabels.at(c).insert(l);
            }
        }
    }
}

void solver::ClusterSolver::sortClusters()
{
    std::function<void(std::shared_ptr<graph::Instance>, std::vector<std::shared_ptr<graph::Instance>>&)> dfs =
        [this, &dfs]
        (std::shared_ptr<graph::Instance> c, std::vector<std::shared_ptr<graph::Instance>>& r)
        {
            for (auto childClusterLabel  : clusterToClusterLabels.at(c))
            {
                auto childCluster = rootLabelToCluster.at(childClusterLabel+1);
                dfs(childCluster, r);
            }
            r.push_back(c);
        };

    std::vector<std::shared_ptr<graph::Instance>> sorted;
    auto rootCluster = rootLabelToCluster.at(0);
    dfs(rootCluster, sorted);
    cluster = sorted;
}

solver::ClusterSolver::ClusterSolver(const std::shared_ptr<graph::Instance>& instance) :
        AbstractSolver(instance),
        clusterRange(std::make_shared<solver::ClusterRange>(this))
{}

bool solver::ClusterSolver::solve()
{
    auto rule = solver::ClusterReductionRule::isApplicable(instance, context);
    clusterReductionRule = std::dynamic_pointer_cast<solver::ClusterReductionRule>(rule);
    if (clusterReductionRule)
    {
        clusterReductionRule->apply();
    }
    else
    {
        cluster = {instance};
        clusterToRootLabel.insert({instance,0});
        rootLabelToCluster.insert({0,instance});
        clusterToClusterLabels.insert({instance,{}});
        return false;
    }
    splitInstance();
    sortClusters();
    return false;
}

void solver::ClusterSolver::unapplyReductions()
{
    if (not clusterReductionRule)
    {
        return;
    }

    mergeCluster();
    clusterReductionRule->unapply();
}

solver::ClusterRange& solver::ClusterSolver::Clusters()
{
    return *clusterRange.get();
}
