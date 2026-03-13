//
// Created by user on 3/8/26.
//

#include "ClusterPointGenerator.hpp"

namespace cluster
{

void ClusterPointGenerator::generateClusterPoints(graph::Node* node) {
    // Recurse
    if (node->leftChild) generateClusterPoints(node->leftChild);
    if (node->rightChild) generateClusterPoints(node->rightChild);
    // Root / Non-Forking / Child Check
    if (node->parent == nullptr) return;
    if (node->leftChild == nullptr or node->rightChild == nullptr) return;
    // We're very strict about potential cluster points having the same depth within the trees.
    // Actually, doing this may save a lot of comparisons later.
    int nodeHeight = checkHeightOfNode(node);
    for (const auto& twin : twinRelation->nodeToTwins[node])
        if (nodeHeight != checkHeightOfNode(twin)) return;
    // Heavy Invariant Checks (Same Leaf Set for class / True Twin Class)
    if (not leafEquivalent(node)) return;
    if (not trueEquivalenceClass(node)) return;

    // No dealbreakers => Cluster Point.
    clusterPoints.push_back(node);
}


ClusterPointGenerator::ClusterPointGenerator(const std::shared_ptr<graph::Instance>& instance, cluster::TwinRelation* twinRelation) {
    this->twinRelation = twinRelation;

    for (const std::shared_ptr<graph::Forest>& forest : *instance)
        rootToForest[forest->Roots().front()] = forest;

    generateClusterPoints(instance->front()->Roots().front());
}


int ClusterPointGenerator::checkHeightOfNode(graph::Node* node)
{
    int distanceToRoot = 0;
    graph::Node* bufferNode = node;

    while ((bufferNode = bufferNode->parent) != nullptr)
        distanceToRoot += 1;

    return distanceToRoot;
}


bool ClusterPointGenerator::trueEquivalenceClass(graph::Node* node)
{
    // Generate the reference set: The cluster point node and its twins.
    std::set<graph::Node*> referenceSet = std::set<graph::Node*>();
    referenceSet.insert(node);
    auto setOfTwins = twinRelation->nodeToTwins[node];
    for (const auto& twin : setOfTwins) referenceSet.insert(twin);

    // Generate comparison set: The union of the twins of the twins, for each twin of the cluster point.
    std::set<graph::Node*> comparisonSet = std::set<graph::Node*>();
    comparisonSet.insert(node);

    for (const auto& twin : setOfTwins)
    {
        comparisonSet.insert(twin);
        for (const auto& twinOfTwin : twinRelation->nodeToTwins[twin])
            comparisonSet.insert(twinOfTwin);
    }

    return referenceSet == comparisonSet;
}


bool ClusterPointGenerator::leafEquivalent(graph::Node* node)
{
    bool isClass = true;

    auto classOfTwins = twinRelation->nodeToTwins[node];
    for (const auto& twin : classOfTwins)
    {
        isClass &= node->hasSameTerminals(twin);

        for (const auto& twinOfTwin : twinRelation->nodeToTwins[twin])
            isClass &= node->hasSameTerminals(twinOfTwin);
    }

    return isClass;
}


ClusterPointGenerator ClusterPointGenerator::wrappedConstructor(const std::shared_ptr<graph::Instance>& instance)
{
    auto temporaryTable = cluster::TwinRelation(instance);
    return ClusterPointGenerator(instance, &temporaryTable);
}

}