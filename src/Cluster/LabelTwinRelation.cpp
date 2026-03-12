//
// Created by user on 3/7/26.
//

#include "LabelTwinRelation.hpp"

namespace cluster
{
LabelTwinRelation::LabelTwinRelation(const std::shared_ptr<graph::Instance>& instance)
{

    std::unordered_map<unsigned int, graph::Node*> labelToTerminal = instance->front()->LabelToTerminal();


    for (const auto keyValue : labelToTerminal )
    {
        auto taxon = keyValue.first;

        // This the object that holds all Node Pointers for a given Taxa.
        std::vector<graph::Node*> taxaClass = std::vector<graph::Node*>();

        // For a given taxa we collect all the nodes that represent it.
        for (const auto &forest : *instance)
        {
            auto terminalToLabel = forest->LabelToTerminal();
            auto leafPointer = terminalToLabel.at(taxon);
            taxaClass.push_back(leafPointer);
        }


        for (auto leafPointer : taxaClass)
        {
            taxaEquivalenceRelation.emplace(leafPointer, taxaClass);
        }
    }



}


std::unordered_map<graph::Node*, std::vector<graph::Node*>>* LabelTwinRelation::getTwins()
{
    return &taxaEquivalenceRelation;
}

}  //namespace graph
