//
// Created by user on 3/8/26.
//

#ifndef PACE2026_CLUSTERPOINTGENERATOR_HPP
#define PACE2026_CLUSTERPOINTGENERATOR_HPP

#include "InteriorTwinRelation.hpp"
namespace graph {





class ClusterPointGenerator
{
  private:
    graph::InteriorTwinRelation* twinRelation;

    void generateClusterPoints(graph::Node* node);

    int checkHeightOfNode(graph::Node* node);

    bool trueEquivalenceClass(graph::Node* node);

  public:

    ClusterPointGenerator(const std::shared_ptr<graph::Instance>& instance, graph::InteriorTwinRelation* twinRelation);

    std::vector<graph::Node*> clusterPoints = std::vector<graph::Node*>();
};


}
#endif  //PACE2026_CLUSTERPOINTGENERATOR_HPP
