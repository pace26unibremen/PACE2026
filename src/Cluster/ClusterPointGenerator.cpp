//
// Created by user on 3/8/26.
//

#include "ClusterPointGenerator.hpp"
#include "functional"
#include "iostream"

namespace cluster
{

void ClusterPointGenerator::generateClusterPoints(graph::Node* node) {

    if (node->leftChild) generateClusterPoints(node->leftChild);
    if (node->rightChild) generateClusterPoints(node->rightChild);


    if (node->parent == nullptr) return;
    if (node->leftChild == nullptr or node->rightChild == nullptr) return;
    int nodeHeight = checkHeightOfNode(node);
    // We're very strict about potential cluster points having the same depth within the trees.
    // Actually, doing this may save a lot of comparisons later.
    for (const auto& twin : twinRelation->nodeToTwins[node])
        if (nodeHeight != checkHeightOfNode(twin)) return;

    if (not trueEquivalenceClass(node) or not leafEquivalent(node)) return;






    clusterPoints.push_back(node);



}


ClusterPointGenerator::ClusterPointGenerator(const std::shared_ptr<graph::Instance>& instance, cluster::InteriorTwinRelation* twinRelation) {
    this->twinRelation = twinRelation;

    for (const std::shared_ptr<graph::Forest>& forest : *instance)
    {
        rootToForest[forest->Roots().front()] = forest;

    }



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
    std::set<graph::Node*> checkSet = std::set<graph::Node*>();

    auto classOfTwins = twinRelation->nodeToTwins[node];


    std::set<graph::Node*> checkSet1 = std::set<graph::Node*>();
    checkSet1.insert(node);
    for (const auto& item : classOfTwins)
    {
        checkSet1.insert(item);
    }

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

    bool sameSet = checkSet == checkSet1;



    return sameSet;
    //return checkSet.size() == twinRelation->nodeToTwins[node].size()+keyCompensator;




}
bool ClusterPointGenerator::leafEquivalent(graph::Node* node)
{

    std::function<void(std::set<unsigned int>*, graph::Node*, std::shared_ptr<graph::Forest> forest)> fetchLabels =
        [&](std::set<unsigned int>* set, graph::Node* node, const std::shared_ptr<graph::Forest>& forest) -> void {

        if (node->leftChild == nullptr && node->rightChild == nullptr)
            set->insert(forest->TerminalToLabel().at(node));

        if (node->leftChild) fetchLabels(set, node->leftChild, forest);
        if (node->rightChild) fetchLabels(set, node->rightChild, forest);
    };


    std::function<std::shared_ptr<graph::Forest>(graph::Node*)> getForestOfNode = [&](graph::Node* node) -> std::shared_ptr<graph::Forest>{
        graph::Node* buffer = node;

        while (buffer->parent != nullptr)
        {
            buffer = buffer->parent;
        }
        return rootToForest[buffer];

    };


    std::set<unsigned int> referenceSet = std::set<unsigned int>();

    auto forest = getForestOfNode(node);
    if (forest == nullptr) return false;
    fetchLabels(&referenceSet, node, forest);

    auto classOfTwins = twinRelation->nodeToTwins[node];


    for (graph::Node* twin : classOfTwins)
    {
        std::set<unsigned int> comparatorSet = std::set<unsigned int>();
        auto twinForest(getForestOfNode(twin));
        if (twinForest == nullptr) return false;
        fetchLabels(&comparatorSet, twin, twinForest);
        if (referenceSet != comparatorSet) return false;
    }

    return true;


}

}