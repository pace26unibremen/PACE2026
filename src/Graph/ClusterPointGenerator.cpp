//
// Created by user on 3/8/26.
//

#include "ClusterPointGenerator.hpp"
namespace graph
{

void ClusterPointGenerator::generateClusterPoints(graph::Node* node) {

    if (node->leftChild) generateClusterPoints(node->leftChild);
    if (node->rightChild) generateClusterPoints(node->rightChild);

    if (node->leftChild == nullptr && node->rightChild == nullptr) return;

    if (not trueEquivalenceClass(node)) return;

    if (node->parent == nullptr) return;

    if (node->leftChild == nullptr || node->rightChild == nullptr) return;

    // We're very strict about potential cluster points having the same depth within the trees.
    int nodeHeight = checkHeightOfNode(node);
    for (const auto& twin : twinRelation->nodeToTwins[node])
    {
        if (nodeHeight != checkHeightOfNode(twin))
            return;
    }


    clusterPoints.push_back(node);



}


ClusterPointGenerator::ClusterPointGenerator(const std::shared_ptr<graph::Instance>& instance, graph::InteriorTwinRelation* twinRelation) {
    this->twinRelation = twinRelation;
    generateClusterPoints(instance->front()->Roots().front());


}
int ClusterPointGenerator::checkHeightOfNode(graph::Node* node)
{
    int distanceToRoot = 0;
    graph::Node* bufferNode = node;

    while (bufferNode->parent != nullptr)
    {
        distanceToRoot += 1;
        bufferNode = bufferNode->parent;
    }

    return distanceToRoot;
}
bool ClusterPointGenerator::trueEquivalenceClass(graph::Node* node)
{
    std::set<Node*> checkSet = std::set<graph::Node*>();

    auto classOfTwins = twinRelation->nodeToTwins[node];


    checkSet.insert(node);

    for (const auto& twin : classOfTwins)
    {
        checkSet.insert(twin);

        auto classOfTwinsOfTwins = twinRelation->nodeToTwins[twin];

        for (const auto& twinOfTwin : classOfTwinsOfTwins)
        {
            checkSet.insert(twinOfTwin);
        }

    }

    int keyCompensator = 1;
    return checkSet.size() == twinRelation->nodeToTwins[node].size()+keyCompensator;




}

}