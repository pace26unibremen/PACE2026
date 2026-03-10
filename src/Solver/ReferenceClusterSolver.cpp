//
// Created by user on 3/7/26.
//

#include "ReferenceClusterSolver.hpp"

#include "../Graph/ClusterInstance.hpp"
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



    while (!instanceStack.empty())
    {
        auto clusterInstances = instanceStack.top();

        for (graph::ClusterInstance& clusterInstance : *clusterInstances)
        {
            for (const auto& lowestCurrentInstance : *clusterInstance.getInstances())
            {
                clusterInstance.decouple();
                auto subSolver = solver::TrivialSolver(lowestCurrentInstance);
                subSolver.solve();
                clusterInstance.couple();
            }


        }

        instanceStack.pop();


    }




    auto finalSolver = solver::BranchingSolver(instance);
    return finalSolver.solve();


    //return std::make_shared<graph::Forest>(instance->at(0)->copy());
}




