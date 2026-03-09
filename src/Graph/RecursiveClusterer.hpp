//
// Created by user on 3/9/26.
//

#ifndef PACE2026_RECURSIVECLUSTERER_HPP
#define PACE2026_RECURSIVECLUSTERER_HPP

#include "Instance.hpp"
#include "stack"
#include "ClusterInstance.hpp"
namespace graph
{




class RecursiveClusterer
{


    void recurseClusters(const std::shared_ptr<std::vector<graph::ClusterInstance>>& clusterInstance);

  public:
    std::stack<std::shared_ptr<std::vector<graph::ClusterInstance>>> instanceStack = std::stack<std::shared_ptr<std::vector<graph::ClusterInstance>>>();
    explicit RecursiveClusterer(const std::shared_ptr<graph::Instance>& instance);









};
}
#endif  //PACE2026_RECURSIVECLUSTERER_HPP
