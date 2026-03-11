//
// Created by user on 3/9/26.
//

#include "RecursiveClusterer.hpp"

#include "ClusterPointGenerator.hpp"

#include <iostream>

namespace graph
{

RecursiveClusterer::RecursiveClusterer(const std::shared_ptr<graph::Instance>& instance) {


    graph::InteriorTwinRelation interiorTwins = graph::InteriorTwinRelation(instance);
    graph::ClusterPointGenerator generator = graph::ClusterPointGenerator(instance, &interiorTwins);
    std::vector<graph::Node*> clusterPoints = generator.clusterPoints;
    graph::ClusterInstance clusterInstance = graph::ClusterInstance(instance, &interiorTwins, &clusterPoints);




    auto instanceVector = std::make_shared<std::vector<graph::ClusterInstance>>(std::vector<graph::ClusterInstance>());


    instanceVector->push_back(clusterInstance);
    if (not instanceVector->empty() and not clusterInstance.getVectorOfInstances()->empty())
    {
        instanceStack.push(instanceVector);
        recurseClusters(instanceVector);
    }


}
void RecursiveClusterer::recurseClusters(const std::shared_ptr<std::vector<graph::ClusterInstance>>& clusterInstances)
{

    static int recursions = 0; recursions += 1; std::cout << "rec : " << recursions << std::endl;

    std::vector<graph::ClusterInstance> recursedInstances = std::vector<graph::ClusterInstance>();

    for (ClusterInstance clusterInstance : *clusterInstances)
    {
        clusterInstance.decouple();



        for (const std::shared_ptr<graph::Instance>& clusterForest : *clusterInstance.getVectorOfInstances())
        {
            graph::InteriorTwinRelation interiorTwins = graph::InteriorTwinRelation(clusterForest);
            graph::ClusterPointGenerator generator = graph::ClusterPointGenerator(clusterForest, &interiorTwins);
            std::vector<graph::Node*> clusterPoints = generator.clusterPoints;
            graph::ClusterInstance newestClusterInstance = graph::ClusterInstance(clusterForest, &interiorTwins, &clusterPoints);

            newestClusterInstance.decouple();

            if (not newestClusterInstance.getVectorOfInstances()->empty())
            {
                recursedInstances.push_back(newestClusterInstance);
            }
        }




    }



    if (not recursedInstances.empty())
    {
        auto pointer = std::make_shared<std::vector<graph::ClusterInstance>>(recursedInstances);
        instanceStack.push(pointer);
        recurseClusters(pointer);
    }




}


}