//
// Created by user on 3/9/26.
//

#include "ClusterInstance.hpp"
#include "InteriorTwinRelation.hpp"
#include "iostream"
namespace graph
{

ClusterInstance::ClusterInstance(const std::shared_ptr<graph::Instance>& instance, graph::InteriorTwinRelation* twinRelation, std::vector<graph::Node*>* clusterPoints)
{

    std::unordered_map<graph::Node*, std::shared_ptr<Forest>> rootToForest = std::unordered_map<graph::Node*, std::shared_ptr<Forest>> ();

    for (const auto& forest : *instance)
        rootToForest[forest->Roots().front()] = forest;


    for (auto clusterPoint : *clusterPoints)
    {
        bool failureDueToOtherClusterPoints = false;
        std::shared_ptr<graph::Instance> clusterInstance = std::make_shared<graph::Instance>(Instance());

        //failureDueToOtherClusterPoints =
            generateClusterForest(&clusterInstance, &rootToForest, clusterPoint);

        auto twins = twinRelation->nodeToTwins[clusterPoint];
        for (auto twin : twins)
        {
            //failureDueToOtherClusterPoints =
                generateClusterForest(&clusterInstance, &rootToForest, twin);
        }

        clusterInstances->push_back(clusterInstance);

        static int cgen = 0; cgen += 1; std::cout << "Successfull ClusterInstance : " << cgen << std::endl;

        return;

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

bool ClusterInstance::generateClusterForest(std::shared_ptr<graph::Instance>* clusterInstance, std::unordered_map<graph::Node*, std::shared_ptr<Forest>>* rootToForest,
                                              graph::Node* node)
{
    Node* forestRootPointer = getClustersRoot(node);

    const std::shared_ptr<Forest>& primeForest = rootToForest->at(forestRootPointer);

    auto shallowCopyNodes = std::make_shared<std::vector<Node>>(primeForest->Nodes());
    auto shallowCopyTerminalToLabel = std::make_shared<std::unordered_map<Node*, unsigned int>>(primeForest->TerminalToLabel());
    auto shallowCopyLabelToTerminal = std::make_shared<std::unordered_map<unsigned int, Node*>>(primeForest->LabelToTerminal());
    auto newRootVector = std::make_shared<std::vector<Node*>>();

    newRootVector->push_back(node);
    Forest primePartialForest = Forest(shallowCopyNodes, shallowCopyTerminalToLabel, shallowCopyLabelToTerminal, newRootVector);

    auto syntheticRoot = std::make_shared<Node>(Node());

    auto clusterForestPointer = std::make_shared<Forest>(primePartialForest);

    graph::ExtendedForestData extendedForestData = ExtendedForestData(node, node->parent, syntheticRoot);

    auto clusterForestDataPointer = std::make_shared<graph::ExtendedForestData>(extendedForestData);

    forestToClusteringData.emplace(clusterForestPointer,clusterForestDataPointer);

    clusterInstance->get()->push_back(clusterForestPointer);

    return false;

}

std::shared_ptr<std::vector<std::shared_ptr<graph::Instance>>> ClusterInstance::getVectorOfInstances() const
{
    return clusterInstances;
}

}