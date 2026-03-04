//
// Created by Philip Kail on 3/4/26.
//

#ifndef PACE2026_PARTIALFOREST_H
#define PACE2026_PARTIALFOREST_H
#include "Forest.hpp"
namespace graph
{

/// \brief A representation of a sub forest that can be disconnected and reconnected from the full instance.
/// It wraps the manipulation of the cluster point and only contains relevant nodes.
class PartialForest : Forest
{
  private:
    /// \brief Pointer to the full forest from which the partial forest came from.
    std::shared_ptr<Forest> outerForest;

    /// \brief Pointer to the original parent node of the given cluster point.
    std::shared_ptr<Node> originalParent;

    /// \brief Pointer to the synthetic root for the clustering.
    std::shared_ptr<Node> syntheticRoot;



  public:
    /// \brief Extended Constructor of Forest. It takes the full forest as a reference and the cluster point to be decoupled.
    /// \note This is where the synthetic root node is created (and will likely be destroyed RAII-style).
    PartialForest(std::shared_ptr<std::vector<Node>> nodes,
                  std::shared_ptr<std::unordered_map<Node*, unsigned int>> terminalToLabel,
                  std::shared_ptr<std::unordered_map<unsigned int, Node*>> labelToTerminal,
                  std::shared_ptr<std::vector<Node*>> roots, std::shared_ptr<Forest> fullForest,
                  std::shared_ptr<Node> clusterPoint);



    /// \brief Decoupling function for the partial forest. Disconnects cluster point from outer tree and sets synthetic root as parent.
    void decoupleForest();

    /// \brief Coupling function for the partial forest. Disconnects cluster point from synthetic and restores original parent.
    void coupleForest();


};

}

#endif  //PACE2026_PARTIALFOREST_H
