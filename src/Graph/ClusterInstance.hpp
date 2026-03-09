//
// Created by user on 3/9/26.
//

#ifndef PACE2026_CLUSTERINSTANCE_HPP
#define PACE2026_CLUSTERINSTANCE_HPP

#include "Instance.hpp"
#include "InteriorTwinRelation.hpp"
#include "Node.hpp"

namespace graph

{

struct ExtendedForestData
{
    Node* clusterNode;
    Node* originalParent;
    std::shared_ptr<Node> syntheticParent;

};


class ClusterInstance
{
  private:
    static graph::Node* getClustersRoot(graph::Node* node);

    bool generateClusterForest(std::shared_ptr<graph::Instance>* clusterInstance, std::unordered_map<graph::Node*, std::shared_ptr<Forest>>* rootToForest, graph::Node* node);

    std::unordered_map<std::shared_ptr<Forest>, std::shared_ptr<ExtendedForestData>> forestToClusteringData =  std::unordered_map<std::shared_ptr<Forest>, std::shared_ptr<ExtendedForestData>>();

    // I'm sane and I can be trusted with C++.
    std::shared_ptr<std::vector<std::shared_ptr<graph::Instance>>> clusterInstances
        = std::make_shared<
            std::vector<std::shared_ptr<graph::Instance>>>(std::vector<std::shared_ptr<graph::Instance>>()
                );



  public:
    ClusterInstance(const std::shared_ptr<graph::Instance>& instance, graph::InteriorTwinRelation* twinRelation, std::vector<graph::Node*>* clusterPoints);


   void couple();

   void decouple();

   std::shared_ptr<std::vector<std::shared_ptr<graph::Instance>>> getInstances() const;


};


}

#endif  //PACE2026_CLUSTERINSTANCE_HPP
