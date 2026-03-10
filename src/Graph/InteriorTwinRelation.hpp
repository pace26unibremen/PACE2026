//
// Created by user on 3/7/26.
//

#ifndef PACE2026_INTERIORTWINRELATION_HPP
#define PACE2026_INTERIORTWINRELATION_HPP

#include "Instance.hpp"
#include "LeastCommonAncestor.hpp"

namespace graph
{

class InteriorTwinRelation
{

  private:
    std::vector<graph::Node*> roots;
    std::vector<std::shared_ptr<graph::LeastCommonAncestor>> LCAs;

    std::unordered_map<graph::Node*, graph::Node*> nodeToTwinBuffer = std::unordered_map<graph::Node*, graph::Node*>();


    void generateInteriorTwinRelation(graph::Node *givenNode, std::shared_ptr<graph::LeastCommonAncestor> homeLCA , std::shared_ptr<graph::LeastCommonAncestor> foreignLCA);

    void initializeTwinVectors(graph::Node* root);

    void fuseTwinBufferToSets();


  public:
    explicit InteriorTwinRelation(const std::shared_ptr<graph::Instance>& instance);

    std::unordered_map<graph::Node*, std::set<graph::Node*>> nodeToTwins = std::unordered_map<graph::Node*, std::set<graph::Node*>>();
};

}

#endif  //PACE2026_INTERIORTWINRELATION_HPP
