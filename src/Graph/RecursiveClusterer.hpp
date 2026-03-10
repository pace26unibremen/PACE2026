//
// Created by user on 3/9/26.
//

#ifndef PACE2026_RECURSIVECLUSTERER_HPP
#define PACE2026_RECURSIVECLUSTERER_HPP

#include "Instance.hpp"
#include "stack"
#include "ClusterInstance.hpp"
namespace graph
{




class RecursiveClusterer
{


    void recurseClusters(const std::shared_ptr<std::vector<graph::ClusterInstance>>& clusterInstance);

  public:
    /// \brief This is a stack that holds all the cluster instances.
    /// \note There are important semantic properties to this stack. It ensures that we only solve the
    /// "youngest generation", the newest instances that have been generated through the recursive clusterer.
    /// This may be beneficial for book-keeping and merging of the clusters, reducing it solely to a primitive coupling,
    /// although this is merely speculation.
    std::stack<std::shared_ptr<std::vector<graph::ClusterInstance>>> instanceStack = std::stack<std::shared_ptr<std::vector<graph::ClusterInstance>>>();

    /// \brief The constructor of the RecursiveClusterer.
    /// \param instance The instance we want to cluster, whereas the clusters of the instance will be
    /// recursively clustered again until no more clusters can be generated.
    explicit RecursiveClusterer(const std::shared_ptr<graph::Instance>& instance);









};
}
#endif  //PACE2026_RECURSIVECLUSTERER_HPP
