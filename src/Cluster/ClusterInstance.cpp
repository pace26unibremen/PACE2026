//
// Created by user on 3/9/26.
//

#include "ClusterInstance.hpp"
#include "TwinRelation.hpp"

namespace cluster
{

ClusterInstance::ClusterInstance(const std::shared_ptr<graph::Instance>& instance, cluster::TwinRelation* twinRelation,
                                 const std::vector<graph::Node*>* clusterPoints)
{

    std::unordered_map<graph::Node*, std::shared_ptr<graph::Forest>> rootToForest = std::unordered_map<graph::Node*, std::shared_ptr<graph::Forest>> ();

    for (const auto& forest : *instance)
        rootToForest[forest->Roots().front()] = forest;

    // Generate an actual instance for each cluster point.
    for (auto clusterPoint : *clusterPoints)
    {
        std::shared_ptr<graph::Instance> clusterInstance = std::make_shared<graph::Instance>(graph::Instance());

        generateClusterForest(&clusterInstance, &rootToForest, clusterPoint);

        for (const auto twin : twinRelation->nodeToTwins[clusterPoint])
            generateClusterForest(&clusterInstance, &rootToForest, twin);


        clusterInstances->push_back(clusterInstance);



    }







}


void ClusterInstance::couple() {
    for (const auto& clusterInstance : *clusterInstances)
    {

        for (const auto& item : *clusterInstance)
        {

            auto data = forestToClusteringData.at(item);
            auto node = data->clusterNode;

            node->parent = data->originalParent;

            if (data->originalParent->leftChild ==  data->syntheticParent.get())
                data->originalParent->leftChild = data->clusterNode;

            if (data -> originalParent->rightChild == data->syntheticParent.get())
                data->originalParent->rightChild = data->clusterNode;
        }





    }


}
void ClusterInstance::decouple() {

    for (const auto& clusterInstance : *clusterInstances)
    {

        for (const auto& item : *clusterInstance)
        {

            auto data = forestToClusteringData.at(item);
            auto node = data->clusterNode;

            node->parent = nullptr;

            if (data->originalParent->leftChild == data->syntheticParent.get())
                data->originalParent->leftChild = data->syntheticParent.get();

            if (data->originalParent->rightChild == data->syntheticParent.get())
                data->originalParent->rightChild = data->syntheticParent.get();
        }

    }

}



graph::Node* ClusterInstance::getClustersRoot(graph::Node* node)
{
    // If the parent is already nullptr then this cluster point is just 1 beneath another cluster point, which has
    // been already decoupled. This means it cannot properly find back to the root blowing up the algorithm.
    if (node->parent == nullptr) return nullptr;

    graph::Node* bufferNode = node;
    while (bufferNode->parent != nullptr)
    {
        bufferNode = bufferNode->parent;
    }
    return bufferNode;
}

// This is quite awful, but then again I'm not gonna invest any further into that as of right now not even knowing
// whether this class will make it to the final solver architecture..
bool ClusterInstance::generateClusterForest(std::shared_ptr<graph::Instance>* clusterInstance, std::unordered_map<graph::Node*, std::shared_ptr<graph::Forest>>* rootToForest,
                                              graph::Node* node)
{
    graph::Node* forestRootPointer = getClustersRoot(node);

    if (forestRootPointer == nullptr)
        throw std::logic_error("The apparent root of a cluster forest is null.");

    const std::shared_ptr<graph::Forest>& primeForest = rootToForest->at(forestRootPointer);

    auto shallowCopyNodes = std::make_shared<std::vector<graph::Node>>(primeForest->Nodes());
    auto shallowCopyTerminalToLabel = std::make_shared<std::unordered_map<graph::Node*, unsigned int>>(primeForest->TerminalToLabel());
    auto shallowCopyLabelToTerminal = std::make_shared<std::unordered_map<unsigned int, graph::Node*>>(primeForest->LabelToTerminal());
    auto newRootVector = std::make_shared<std::vector<graph::Node*>>();

    newRootVector->push_back(node);
    graph::Forest primePartialForest = graph::Forest(shallowCopyNodes, shallowCopyTerminalToLabel, shallowCopyLabelToTerminal, newRootVector);

    auto syntheticLeaf = std::make_shared<graph::Node>(graph::Node());

    auto clusterForestPointer = std::make_shared<graph::Forest>(primePartialForest);

    cluster::ExtendedForestData extendedForestData = ExtendedForestData(node, node->parent, syntheticLeaf);

    auto clusterForestDataPointer = std::make_shared<cluster::ExtendedForestData>(extendedForestData);

    forestToClusteringData.emplace(clusterForestPointer,clusterForestDataPointer);

    clusterInstance->get()->push_back(clusterForestPointer);

    return false;

}

std::shared_ptr<std::vector<std::shared_ptr<graph::Instance>>> ClusterInstance::getVectorOfInstances() const
{
    return clusterInstances;
}

}