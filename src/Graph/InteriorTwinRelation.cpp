//
// Created by user on 3/7/26.
//

#include "InteriorTwinRelation.hpp"

#include "LabelTwinRelation.hpp"
#include "LeastCommonAncestor.hpp"

#include <iostream>

namespace graph
{


InteriorTwinRelation::InteriorTwinRelation(const std::shared_ptr<graph::Instance>& instance) {


    graph::LabelTwinRelation labelTwins = graph::LabelTwinRelation(instance);

    auto map = labelTwins.getTwins();

    for (const auto& pair : *map)
    {
        auto node = pair.first;
        auto list = pair.second;

        for (const auto& item : list)
        {
            if (node == item) continue;
            nodeToTwins[node].insert(item);
        }


    }



    roots = std::vector<graph::Node*>();
    LCAs = std::vector<std::shared_ptr<graph::LeastCommonAncestor>>();


    for (auto& forest : *instance)
    {
        auto firstRoot = forest->Roots().front();
        roots.push_back(firstRoot);

        auto LCA = std::make_shared<graph::LeastCommonAncestor>(graph::LeastCommonAncestor((forest)));
        LCAs.push_back(LCA);
    }

    // This may have accidentally escalated the algorithms run time from linear to n^2 but if you can see whats going
    // on here with your inner eye it's pretty fucking beautiful.
    // This will likely be optimized back into linear time.

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


void InteriorTwinRelation::generateInteriorTwinRelation(graph::Node *givenNode, std::shared_ptr<graph::LeastCommonAncestor> homeLCA, std::shared_ptr<graph::LeastCommonAncestor> foreignLCA) {
    if (roots.size() != LCAs.size())
    {
        std::cerr << "Something has gone fatally wrong within generateInteriorTwinRelation" << std::endl;
        return;
    }

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
void InteriorTwinRelation::initializeTwinVectors(graph::Node* root) {
    if (auto leftChild = root->leftChild)
    {
        std::vector<graph::Node*> twinListLeft = std::vector<graph::Node*>();
        //nodeToTwins.emplace(leftChild,twinListLeft);
        initializeTwinVectors(leftChild);
    }

    if (auto rightChild = root->rightChild)
    {
        std::vector<graph::Node*> twinListRight = std::vector<graph::Node*>();
        //nodeToTwins.emplace(rightChild,twinListRight);
        initializeTwinVectors(rightChild);
    }


}
void InteriorTwinRelation::fuseTwinBufferToSets() {
    for (const auto& pair : nodeToTwinBuffer)
    {
        nodeToTwins[pair.first].insert(pair.second);
    }
    nodeToTwinBuffer.clear();
}

}