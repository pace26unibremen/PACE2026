//
// Created by user on 3/9/26.
//

#ifndef PACE2026_CLUSTERINSTANCE_HPP
#define PACE2026_CLUSTERINSTANCE_HPP

#include "../Graph/Instance.hpp"
#include "../Graph/Node.hpp"
#include "TwinRelation.hpp"

namespace cluster

{

/// \brief ExtendedForestData struct. Preliminary measure to be able to glue a cluster and it's outer forest back
/// together.
struct ExtendedForestData
{
    graph::Node* clusterNode;
    graph::Node* originalParent;
    std::shared_ptr<graph::Node> syntheticParent;

};

/// \brief ClusterInstance class. Preliminary measure to test whether the clustering even works in the slightest.
class ClusterInstance
{
  private:
    static graph::Node* getClustersRoot(graph::Node* node);

    bool generateClusterForest(std::shared_ptr<graph::Instance>* clusterInstance, std::unordered_map<graph::Node*, std::shared_ptr<graph::Forest>>* rootToForest, graph::Node* node);

    std::unordered_map<std::shared_ptr<graph::Forest>, std::shared_ptr<ExtendedForestData>> forestToClusteringData =  std::unordered_map<std::shared_ptr<graph::Forest>, std::shared_ptr<ExtendedForestData>>();

    // I'm sane and I can be trusted with C++.
    std::shared_ptr<std::vector<std::shared_ptr<graph::Instance>>> clusterInstances
        = std::make_shared<
            std::vector<std::shared_ptr<graph::Instance>>>(std::vector<std::shared_ptr<graph::Instance>>()
                );



  public:
    /// \brief This is the constructor of a ClusterInstance.
    /// \param instance The instance we want to partition into smaller/cluster trees.
    /// \param twinRelation The Twin-Map to get the equivalence class of a node to discern where to split each forest
    /// of the instance.
    /// \param clusterPoints The cluster points of the instance we want to split up.
    ClusterInstance(const std::shared_ptr<graph::Instance>& instance, cluster::TwinRelation* twinRelation, std::vector<graph::Node*>* clusterPoints);

    /// \brief This is a wrapped constructor that skips dealing with all the prior classes.
    /// Be careful when using it, it'll keep the data structures used internally.
    static ClusterInstance wrappedConstructor(const std::shared_ptr<graph::Instance>& instance);
    /// \brief This function separates a cluster forest from their parent/outer tree.
    void couple();
    /// \brief This function reconnects a cluster forest to their parent/outer tree. When decoupling, one must ensure
    /// to couple again in the future lest disaster will ensure.
    void decouple();

    /// \brief This function returns a shared pointer to the vector of cluster instances that have been generated
    /// by splitting up each instance using the clusterPoints and the twinRelation.
    /// \note This vector represents the cluster partitions. This does NOT account for their height, its unordered.
   std::shared_ptr<std::vector<std::shared_ptr<graph::Instance>>> getVectorOfInstances() const;


};


}

#endif  //PACE2026_CLUSTERINSTANCE_HPP
