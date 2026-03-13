//
// Created by user on 3/7/26.
//

#include "TwinRelation.hpp"

#include "LeastCommonAncestor.hpp"


namespace cluster
{

TwinRelation::TwinRelation(const std::shared_ptr<graph::Instance>& instance) {

    initializeLeafTable(instance);

    roots = std::vector<graph::Node*>();
    LCAs = std::vector<std::shared_ptr<cluster::LeastCommonAncestor>>();


    for (auto& forest : *instance)
    {
        auto firstRoot = forest->Roots().front();
        roots.push_back(firstRoot);

        auto LCA = std::make_shared<cluster::LeastCommonAncestor>(cluster::LeastCommonAncestor((forest)));
        LCAs.push_back(LCA);
    }

    // This likely escalates runtime from linear to square. However, this is likely to be unavoidable.

    for (int i = 0; i < roots.size(); ++i)
    {

        for (int j = 0; j < LCAs.size(); ++j)
        {
            if (i == j) continue;
            generateInteriorTwinRelation(roots.at(i), LCAs.at(i) , LCAs.at(j));
            fuseTwinBufferToSets();
        }
    }


}


void TwinRelation::generateInteriorTwinRelation(graph::Node *givenNode,
                                                        const std::shared_ptr<cluster::LeastCommonAncestor>& homeLCA,
                                                        const std::shared_ptr<cluster::LeastCommonAncestor>& foreignLCA) {
    if (roots.size() != LCAs.size())
    {
        throw std::runtime_error("The size of the Roots and the LCAs is different, implying initializing error."
                                 "Is this run from within a reduction rule?.");
    }
    // This scurrilous construction is a remnant of the reimplementation of rSPR. It will be changed in the future.
    std::vector<graph::Node*> children = std::vector<graph::Node*>();

    if (auto child = givenNode->leftChild) children.push_back(child);
    if (auto child = givenNode->rightChild) children.push_back(child);

    if (children.empty()) return;

    unsigned int index = 0;

    generateInteriorTwinRelation(children.at(index), homeLCA, foreignLCA);

    nodeToTwinBuffer[givenNode] = nodeToTwinBuffer[children.at(index)];

    index += 1;

    if (index < children.size())
    {
        generateInteriorTwinRelation(children.at(index), homeLCA, foreignLCA);
        graph::Node* twin = foreignLCA->getLeastCommonAncestor(
            homeLCA->getNodesToPreorderNumber()->at(givenNode),
            homeLCA->getNodesToPreorderNumber()->at(children.at(index))
            );
        nodeToTwinBuffer[givenNode] = twin;
    }
}





void TwinRelation::fuseTwinBufferToSets() {
    for (const auto& pair : nodeToTwinBuffer)
        nodeToTwins[pair.first].insert(pair.second);

    nodeToTwinBuffer.clear();
}

void TwinRelation::initializeLeafTable(const std::shared_ptr<graph::Instance>& instance) {

    std::unordered_map<unsigned int, graph::Node*> labelToTerminal = instance->front()->LabelToTerminal();
    for (const auto keyValue : labelToTerminal )
    {
        auto label = keyValue.first;

        // This the object that holds all Node Pointers for a given Taxa.
        std::set<graph::Node*> allNodesFromLabel = std::set<graph::Node*>();

        // For a given label we collect all the nodes that represent it.
        for (const auto &forest : *instance)
            allNodesFromLabel.insert(forest->LabelToTerminal().at(label));


        for (auto key : allNodesFromLabel)
        {
            std::set<graph::Node*> allOtherNodes = allNodesFromLabel;
            allOtherNodes.erase(key);
            nodeToTwins[key] = allOtherNodes;
        }

    }
}


}