//
// Created by user on 3/7/26.
//

#ifndef PACE2026_LABELTWINRELATION_HPP
#define PACE2026_LABELTWINRELATION_HPP

#include "Instance.hpp"


namespace graph
{

class LabelTwinRelation
{
  private:
    std::unordered_map<graph::Node*, std::vector<graph::Node*>> taxaEquivalenceRelation = std::unordered_map<graph::Node*, std::vector<graph::Node*>>();

  public:
    explicit LabelTwinRelation(const std::shared_ptr<graph::Instance>& instance);


    std::unordered_map<graph::Node*, std::vector<graph::Node*>>* getTwins();



};

}  //namespace graph

#endif  //PACE2026_LABELTWINRELATION_HPP
