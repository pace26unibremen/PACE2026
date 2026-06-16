#ifndef PACE2026_CLUSTER_SOLVER_HPP
#define PACE2026_CLUSTER_SOLVER_HPP

#include "../AbstractSolver.hpp"
#include "../Context.hpp"
#include "../Action/DecoupleSubtreeAction.hpp"

#include <list>
#include <stack>

namespace solver
{

class ClusterRange;

class ClusterSolver : public AbstractSolver
{
    friend ClusterRange;

    std::shared_ptr<ClusterRange> clusterRange;

    std::vector<std::shared_ptr<graph::Instance>> cluster;

    const std::shared_ptr<solver::Context> context = std::make_shared<solver::Context>();

    /// \brief A collection of all cluster roots that have been cutted.
    std::unordered_set<unsigned int> cuttedClusterRoots;

    /// \brief the cluster reduction rule
    // std::shared_ptr<solver::ClusterReductionRule> clusterReductionRule;

    std::list<std::list<std::pair<std::shared_ptr<graph::Forest>, graph::Node*>>> pointsAndForests_PerCluster =
        std::list<std::list<std::pair<std::shared_ptr<graph::Forest>, graph::Node*>>>();

    /// \brief Maps a cluster to the label that annotates its root.
    /// The root cluster without an annotated root maps to 0
    std::unordered_map<std::shared_ptr<graph::Instance>, unsigned int> clusterToRootLabel;

    /// \brief Maps the label that annotates a cluster root to the corresponding cluster.
    /// 0 maps to the root cluster.
    std::unordered_map<unsigned int, std::shared_ptr<graph::Instance>> rootLabelToCluster;

    std::unordered_map<std::shared_ptr<graph::Instance>, std::unordered_set<unsigned int>> clusterToClusterLabels;

    std::stack<DecoupleSubtreeAction> changes = std::stack<DecoupleSubtreeAction>();

    std::stack<DecoupleSubtreeAction> changesOnF0 = std::stack<DecoupleSubtreeAction>();

    std::shared_ptr<graph::Instance> buildSingleCluster(unsigned int i);

    void getClusterPoints();

    void resizeSubtreeTerminalsVector();

    void decoupleSubtrees();

    void splitInstance();

    void sortClusters();

    void mergeCluster();

    void coupleSubtrees();

    void collectCuttedClusterRoot(unsigned int index);

    void cutClusterTerminals(unsigned int index);

  public:
    explicit ClusterSolver(const std::shared_ptr<graph::Instance>& instance);

    ~ClusterSolver() override = default;

    bool solve() override;

    void unapplyReductions() override;

    ClusterRange& Clusters();
};

}  //namespace solver

#endif  //PACE2026_CLUSTER_SOLVER_HPP
