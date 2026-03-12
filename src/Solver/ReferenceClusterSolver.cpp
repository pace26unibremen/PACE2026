//
// Created by user on 3/7/26.
//

#include "ReferenceClusterSolver.hpp"

#include "../Cluster/ClusterInstance.hpp"
#include "../Cluster/RecursiveClusterer.hpp"
#include "BranchingSolver.hpp"
#include "TrivialSolver.hpp"

#include <iostream>

solver::ReferenceClusterSolver::ReferenceClusterSolver(const std::shared_ptr<graph::Instance>& instance) :
        AbstractSolver(instance)
{}
std::shared_ptr<graph::Forest> solver::ReferenceClusterSolver::solve()
{

    cluster::RecursiveClusterer clusterStack = cluster::RecursiveClusterer(instance);


    auto instanceStack = clusterStack.instanceStack;

    std::cout << "Recursive Cluster Size: " << instanceStack.size() << std::endl;


    while (!instanceStack.empty())
    {
        auto clusterInstances = instanceStack.top();

        for (cluster::ClusterInstance& clusterInstance : *clusterInstances)
        {
            for (const auto& lowestCurrentInstance : *clusterInstance.getVectorOfInstances())
            {
                clusterInstance.decouple();
                auto subSolver = solver::TrivialSolver(lowestCurrentInstance);
                //subSolver.solve();
                clusterInstance.couple();
            }


        }

        instanceStack.pop();


    }




    auto finalSolver = solver::BranchingSolver(instance);
    //return finalSolver.solve();


    return std::make_shared<graph::Forest>(instance->at(0)->copy());
}




