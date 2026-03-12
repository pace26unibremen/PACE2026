//
// Created by user on 3/8/26.
//

#ifndef PACE2026_CLUSTERPOINTGENERATOR_HPP
#define PACE2026_CLUSTERPOINTGENERATOR_HPP

#include "InteriorTwinRelation.hpp"
namespace cluster {





class ClusterPointGenerator
{
  private:
    cluster::InteriorTwinRelation* twinRelation;
    std::unordered_map<graph::Node*, std::shared_ptr<graph::Forest>> rootToForest = std::unordered_map<graph::Node*, std::shared_ptr<graph::Forest>> ();


    void generateClusterPoints(graph::Node* node);

    int checkHeightOfNode(graph::Node* node);

    bool trueEquivalenceClass(graph::Node* node);

    bool leafEquivalent(graph::Node* node);



  public:
    /// \brief This is the constructor of the ClusterPointGenerator.
    /// \param instance The instance for which we want to generate cluster points for.
    /// \param twinRelation The table that links each note to their corresponding twins.
    /// \note A node is a cluster point iff they're: not the root, not a leaf, have both children
    /// & are in a true equivalence class.
    ClusterPointGenerator(const std::shared_ptr<graph::Instance>& instance, cluster::InteriorTwinRelation* twinRelation);

    /// \brief This static function returns an instance of the ClusterPointGenerator without the hassle of having to
    /// generate the Twin-Table object first. The table will be generated internally and discarded after usage, which
    /// has to be kept in mind when using this function. (Potential time and memory cost!)
    /// \note Access to Cluster Point vector through public member access.
    static ClusterPointGenerator wrappedConstructor(const std::shared_ptr<graph::Instance>& instance);

    /// \brief This is the vector that contains the cluster points of a given instance. All of the points within
    /// this vector are from the front-tree of the instance and in order to use them one must fetch the twins of a node
    /// within this vector and fetch the twins of the clusterpoint through the Twin-Map.
    std::vector<graph::Node*> clusterPoints = std::vector<graph::Node*>();
};


}
#endif  //PACE2026_CLUSTERPOINTGENERATOR_HPP
