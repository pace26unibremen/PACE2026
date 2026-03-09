//
// Created by user on 3/7/26.
//

#include "ReferenceClusterSolver.hpp"

#include "../Graph/ClusterInstance.hpp"
#include "../Graph/ClusterPointGenerator.hpp"
#include "../Graph/InteriorTwinRelation.hpp"
#include "../Graph/LabelTwinRelation.hpp"
#include "../Graph/RecursiveClusterer.hpp"
#include "BranchingSolver.hpp"

#include <iostream>
#include "TrivialSolver.hpp"

solver::ReferenceClusterSolver::ReferenceClusterSolver(const std::shared_ptr<graph::Instance>& instance) :
        AbstractSolver(instance)
{}
std::shared_ptr<graph::Forest> solver::ReferenceClusterSolver::solve()
{

    graph::RecursiveClusterer clusterStack = graph::RecursiveClusterer(instance);


    auto instanceStack = clusterStack.instanceStack;


    /*
    while (!instanceStack.empty())
    {
        auto clusterInstances = instanceStack.top();

        for (graph::ClusterInstance& clusterInstance : *clusterInstances)
        {
            for (const auto& jesus : *clusterInstance.getInstances())
            {
                clusterInstance.decouple();
                auto subSolver = solver::TrivialSolver(jesus);
                subSolver.solve();
                clusterInstance.couple();
            }


        }

        instanceStack.pop();


    }

     */


    auto finalSolver = solver::BranchingSolver(instance);
    finalSolver.solve();


    return std::make_shared<graph::Forest>(instance->at(0)->copy());
}


void solver::ReferenceClusterSolver::getAllNodePointersOfAForest(graph::Node* node, std::set<graph::Node*>* set) {
    set->insert(node);

    if (node->leftChild)
        getAllNodePointersOfAForest(node->leftChild, set);

    if (node->rightChild)
        getAllNodePointersOfAForest(node->rightChild, set);

}

