#include "ClusterSolver.hpp"

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
        AbstractSolver(instance)
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
        return false;
    }

    splitInstance();
    sortClusters();


    std::unordered_set<unsigned int> cuttedClusterRoots;

    bool solved = false;
    for (unsigned int i = 0; i < cluster.size(); ++i)
    {
        auto c = cluster.at(i);

        graph::DotInstance(c, "../../res/cluster_" + std::to_string(i) + "_initial.dot"); // Todo
        graph::WriteInstance(c, "../../res/cluster_" + std::to_string(i) + "_initial.nw"); // Todo

        auto config = std::make_shared<solver::BranchingSolverConfiguration>();
        subSolver.emplace_back(c, config);
        auto& subS = subSolver[i];
        // configuration->debPlugin = {std::make_shared<DebugPlugin>("../../res/cluster_" + std::to_string(i) + "_bs/")}; // Todo

        unsigned int clusterRootLabel = clusterToRootLabel.at(c);

        if (clusterRootLabel != 0)
        {
            // std::cout << i << ": add cluster root label to context (" << clusterRootLabel << ")" << std::endl; // Todo
            subS.GetContext()->clusterRoot.insert(clusterRootLabel); // Todo
            subS.GetContext()->clusterRootLabel = clusterRootLabel;
        }

        std::unordered_set<unsigned int> cutClusterTerminals;
        for (auto l : clusterToClusterLabels.at(c))
        {
            if (cuttedClusterRoots.contains(l+1))
            {
                cutClusterTerminals.insert(l);
            }
        }
        if (not cutClusterTerminals.empty())
        {
            auto r = solver::SingleVertexTreePropagationRule(c,context,cutClusterTerminals);
            r.apply();
        }
        // graph::DotInstance(c, "../../res/cluster_" + std::to_string(i) + "_removedClusterLabel.dot"); // Todo

        solved &= subS.solve();

        // graph::DotInstance(c, "../../res/cluster_" + std::to_string(i) + "_solved.dot"); // Todo

        if (clusterRootLabel != 0)
        {
            auto clusterRootNode = c->at(0)->LabelToTerminal().at(clusterRootLabel);
            if (clusterRootNode->isTrueTerminal() and not clusterRootNode->parent)
            {
                cuttedClusterRoots.insert(clusterRootLabel);
                // std::cout << i << ": cluster root (" << clusterRootLabel << ") cutted" << std::endl; // Todo
            }
        }
    }
    return solved;
}

void solver::ClusterSolver::unapplyReductions()
{
    if (not clusterReductionRule)
    {
        return;
    }

    for (unsigned int i= 0; i < cluster.size(); i++)
    {
        subSolver[i].unapplyReductions();
        // graph::DotInstance(cluster[i], "../../res/cluster_" + std::to_string(i) + "_unapplyReductions.dot"); // Todo
    }

    mergeCluster();
    // graph::DotInstance(instance, "../../res/merged_cluster.dot"); // Todo
    clusterReductionRule->unapply();
    // graph::DotInstance(instance, "../../res/unapplied_crr.dot"); // Todo
}

const std::vector<std::shared_ptr<graph::Instance>>& solver::ClusterSolver::SubProblems() const
{
    return cluster;
}
