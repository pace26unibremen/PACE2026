//
// Created by user on 3/7/26.
//

#ifndef PACE2026_INTERIORTWINRELATION_HPP
#define PACE2026_INTERIORTWINRELATION_HPP

#include "../Graph/Instance.hpp"
#include "LeastCommonAncestor.hpp"

namespace cluster
{

class InteriorTwinRelation
{

  private:
    std::vector<graph::Node*> roots;
    std::vector<std::shared_ptr<cluster::LeastCommonAncestor>> LCAs;

    std::unordered_map<graph::Node*, graph::Node*> nodeToTwinBuffer = std::unordered_map<graph::Node*, graph::Node*>();


    void generateInteriorTwinRelation(graph::Node *givenNode, const std::shared_ptr<cluster::LeastCommonAncestor>& homeLCA , const std::shared_ptr<cluster::LeastCommonAncestor>& foreignLCA);

    void initializeTwinVectors(graph::Node* root);

    void fuseTwinBufferToSets();


  public:
    /// \brief Returns a pointer to a map which maps each node to their twins.
    /// This list also encompasses the twins. Be careful, the size of a leaf-twin set is larger than an interior node
    /// twin set.
    explicit InteriorTwinRelation(const std::shared_ptr<graph::Instance>& instance);

    /// \brief Map that maps each node of the entire instance to their corresponding twins. You should be enormously
    /// careful when using this map, because not all entries are true equivalence classes.
    /// \note A true equivalence class (the intersection of a node with all the twins and the twins of the twins being
    /// the twins of the node) is also a cluster point. Not all nodes are in a true equivalence class.
    /// Also, the term "true equivalence class" is made up.
    std::unordered_map<graph::Node*, std::set<graph::Node*>> nodeToTwins = std::unordered_map<graph::Node*, std::set<graph::Node*>>();
};

}

#endif  //PACE2026_INTERIORTWINRELATION_HPP
