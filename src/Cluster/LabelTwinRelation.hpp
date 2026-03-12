//
// Created by user on 3/7/26.
//

#ifndef PACE2026_LABELTWINRELATION_HPP
#define PACE2026_LABELTWINRELATION_HPP

#include "../Graph/Instance.hpp"

namespace cluster
{

/// \brief This class provides generation and access to a table that links each leaf twin.
class LabelTwinRelation
{
  private:
    std::unordered_map<graph::Node*, std::vector<graph::Node*>> taxaEquivalenceRelation = std::unordered_map<graph::Node*, std::vector<graph::Node*>>();

  public:

    /// \brief Creates a map that maps each leaf with a label to all the other nodes of the instance which share the
    /// same label.
    /// \param instance the intance to generate the leaf-twin table from.
    explicit LabelTwinRelation(const std::shared_ptr<graph::Instance>& instance);

    /// \brief Returns a pointer to a map which maps each leaf node to their leaf twins. (Taxa-Equivalent Nodes)
    std::unordered_map<graph::Node*, std::vector<graph::Node*>>* getTwins();



};

}  //namespace graph

#endif  //PACE2026_LABELTWINRELATION_HPP
