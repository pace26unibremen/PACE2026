//
// Created by user on 3/9/26.
//

#include "RecursiveClusterer.hpp"

#include "ClusterPointGenerator.hpp"

#include <iostream>

namespace cluster
{

RecursiveClusterer::RecursiveClusterer(const std::shared_ptr<graph::Instance>& instance) {


    cluster::TwinRelation interiorTwins = cluster::TwinRelation(instance);
    cluster::ClusterPointGenerator generator = cluster::ClusterPointGenerator(instance, &interiorTwins);
    std::vector<graph::Node*> clusterPoints = generator.clusterPoints;
    cluster::ClusterInstance clusterInstance = cluster::ClusterInstance(instance, &interiorTwins, &clusterPoints);




    auto instanceVector = std::make_shared<std::vector<cluster::ClusterInstance>>(std::vector<cluster::ClusterInstance>());


    instanceVector->push_back(clusterInstance);
    if (not instanceVector->empty() and not clusterInstance.getVectorOfInstances()->empty())
    {
        instanceStack.push(instanceVector);
        recurseClusters(instanceVector);
    }


}
void RecursiveClusterer::recurseClusters(const std::shared_ptr<std::vector<cluster::ClusterInstance>>& clusterInstances)
{
    std::vector<cluster::ClusterInstance> recursedInstances = std::vector<cluster::ClusterInstance>();

    for (cluster::ClusterInstance clusterInstance : *clusterInstances)
    {
        clusterInstance.decouple();



        for (const std::shared_ptr<graph::Instance>& clusterForest : *clusterInstance.getVectorOfInstances())
        {
            cluster::TwinRelation interiorTwins = cluster::TwinRelation(clusterForest);
            cluster::ClusterPointGenerator generator = cluster::ClusterPointGenerator(clusterForest, &interiorTwins);
            std::vector<graph::Node*> clusterPoints = generator.clusterPoints;
            cluster::ClusterInstance newestClusterInstance = cluster::ClusterInstance(clusterForest, &interiorTwins, &clusterPoints);

            newestClusterInstance.decouple();

            if (not newestClusterInstance.getVectorOfInstances()->empty())
            {
                recursedInstances.push_back(newestClusterInstance);
            }
        }




    }



    if (not recursedInstances.empty())
    {
        auto pointer = std::make_shared<std::vector<cluster::ClusterInstance>>(recursedInstances);
        instanceStack.push(pointer);
        recurseClusters(pointer);
    }




}


}