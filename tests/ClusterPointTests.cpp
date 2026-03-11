//
// Created by user on 3/10/26.
//
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/ClusterPointGenerator.hpp"
#include "../src/Graph/Forest.hpp"
#include "../src/Graph/Instance.hpp"
#include "../src/Graph/InteriorTwinRelation.hpp"
#include "../src/Graph/RecursiveClusterer.hpp"
#include "functional"
#include "iostream"




int labelMismatchTestFunction(std::shared_ptr<graph::Instance>*  instance, graph::InteriorTwinRelation* interiorTwins,
                              graph::ClusterPointGenerator* generator)
{
    int labelMismatches = 0;

    std::vector<graph::Node*> clusterPoints = generator->clusterPoints;



    std::function<void(std::set<unsigned int>*, graph::Node*, graph::Forest* forest)> fetchLabels =
        [&](std::set<unsigned int>* set, graph::Node* node, graph::Forest* forest) -> void {

        if (node->leftChild == nullptr && node->rightChild == nullptr)
            set->insert(forest->TerminalToLabel().at(node));

        // This property is silly, but important: If a forest cluster child is decoupled and we run this label Test,
        // then the leaves should not be in the Map of the forest with the decoupled child.
        if (not forest->TerminalToLabel().contains(node) && node->leftChild == nullptr && node->rightChild == nullptr)
        {
            set->insert(1000);
        }

        if (node->leftChild) fetchLabels(set, node->leftChild, forest);
        if (node->rightChild) fetchLabels(set, node->rightChild, forest);
    };


    std::function<std::shared_ptr<graph::Forest>(graph::Node*)> getForestOfNode = [&](graph::Node* node) -> std::shared_ptr<graph::Forest>{
        graph::Node* buffer = node;
        while (buffer->parent != nullptr) buffer = buffer->parent;

        for (const auto& forest : **instance)
        {
            if (forest->Roots().front() == buffer) return forest;
        }
        return nullptr;

    };



    for (auto clusterPoint : clusterPoints)
    {
        std::set<unsigned int> referenceSet = std::set<unsigned int>();



        fetchLabels(&referenceSet, clusterPoint, getForestOfNode(clusterPoint).get());


        for (const auto& twin : interiorTwins->nodeToTwins[clusterPoint])
        {
            std::set<unsigned int> comparisonSet = std::set<unsigned int>();

            fetchLabels(&comparisonSet, twin, getForestOfNode(twin).get());

            for (const auto& label : comparisonSet)
            {
                if (!referenceSet.contains(label) || referenceSet.size() != comparisonSet.size() ||
                    comparisonSet.contains(1000) )
                {
                    labelMismatches += 1;
                }
            }


        }


        referenceSet.clear();

    }
    return labelMismatches;
}

size_t rootIsClusterPoint(std::shared_ptr<graph::Instance>*  instance, graph::InteriorTwinRelation* interiorTwins,
                              graph::ClusterPointGenerator* generator)
{
    std::vector<graph::Node*> clusterPoints = generator->clusterPoints;

    std::set<graph::Node*> testSet = std::set<graph::Node*>();
    for (const auto& forest : **instance){

        auto root = forest->Roots().front();
        for (const auto& twinOfRoot : interiorTwins->nodeToTwins[root])
        {
            testSet.insert(twinOfRoot);


            for (auto twinOfTwin : interiorTwins->nodeToTwins[twinOfRoot])
            {
                testSet.insert(twinOfTwin);
            }

        }


    }

    // This counts the number of "foreign nodes" in the root twin test set.
    int testSetContaminator = 0;
    for (auto forest : **instance)
    {
        auto root = forest->Roots().front();
        if (!testSet.contains(root))
            testSetContaminator += 1;
    }

    return testSet.size()+testSetContaminator;
}


// Let R be the set of labels/leafs from a given cluster point from the list of cluster points (which belong to the same
// forest).  (Reference).
// Let C be the set of a twin of the cluster point (Comparator).
// Test Question: For each x in C : If x in C then x in R AND Size of C = Size of R. (If not, fail test.)
TEST_CASE("ClusterLabelEquivalence")
{
    SECTION("Check if the labels of a cluster point are the same across all clusters.")
    {


        std::shared_ptr<graph::Instance>  instance = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_100_11_stressTest.tree");
        graph::InteriorTwinRelation interiorTwins = graph::InteriorTwinRelation(instance);
        graph::ClusterPointGenerator generator = graph::ClusterPointGenerator(instance, &interiorTwins);

        int labelMismatches = labelMismatchTestFunction(&instance, &interiorTwins, &generator);



        REQUIRE(labelMismatches == 0);
    }


}

// Theoretically, a cluster gets its own designated set of roots. An instance itself should be a cluster and be solved
// as one such. If that were not the case then clustering shouldn't be able to work to begin with.
// The roots of the cluster used to be interior nodes of its outer tree.
// TL;DR: The roots of any given instance must be a viable cluster point.
TEST_CASE("RootTwinEquivalence")
{
    SECTION("Check if the twins of the roots are the other roots. .")
    {
        std::shared_ptr<graph::Instance>  instance = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_100_11_stressTest.tree");
        
        graph::InteriorTwinRelation interiorTwins = graph::InteriorTwinRelation(instance);
        graph::ClusterPointGenerator generator = graph::ClusterPointGenerator(instance, &interiorTwins);

        size_t rootClassSize = rootIsClusterPoint(&instance, &interiorTwins, &generator);

        // This is a bit hacky but should suffice for the test...
        REQUIRE(rootClassSize == instance->size());
    }

}
// This replicates the tests above (Single Cluster / Head Instance) for each Cluster Instance.
// This may be absurdly over convoluted because it seems like the LCA Method already generates all possible cluster
// points.
TEST_CASE("RecursiveClusterI")
{
    SECTION("Tests on recursive clustering.")
    {
        std::shared_ptr<graph::Instance>  instance = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_100_11_stressTest.tree");

        graph::InteriorTwinRelation interiorTwins = graph::InteriorTwinRelation(instance);
        graph::ClusterPointGenerator generator = graph::ClusterPointGenerator(instance, &interiorTwins);
        std::vector<graph::Node*> clusterPoints = generator.clusterPoints;

        int labelMismatches = 0;
        int rootClusterClassErrors = 0;

        labelMismatches += labelMismatchTestFunction(&instance, &interiorTwins, &generator);
        if  (rootIsClusterPoint(&instance, &interiorTwins, &generator) != instance->size()) rootClusterClassErrors += 1;


        auto recursiveClusterer = graph::RecursiveClusterer(instance);
        auto instanceStack = recursiveClusterer.instanceStack;

        while (!instanceStack.empty())
        {
            auto clusterInstances = instanceStack.top();

            for (graph::ClusterInstance& clusterInstance : *clusterInstances)
            {
                for (const auto& lowestCurrentInstance : *clusterInstance.getInstances())
                {
                    clusterInstance.decouple();

                    auto x = clusterInstance.getInstances();

                    for (auto subInstance : *x)
                    {
                        graph::InteriorTwinRelation interiorTwinsCluster = graph::InteriorTwinRelation(instance);
                        graph::ClusterPointGenerator generatorCluster = graph::ClusterPointGenerator(instance, &interiorTwins);

                        labelMismatches  += labelMismatchTestFunction(&subInstance, &interiorTwinsCluster, &generatorCluster);
                        size_t rootClassSize = rootIsClusterPoint(&subInstance, &interiorTwinsCluster, &generatorCluster);

                        if (rootClassSize != subInstance->size()) rootClusterClassErrors += 1;

                    }


                    //clusterInstance.couple();
                }

            }
            instanceStack.pop();
        }


        bool success = (rootClusterClassErrors == 0) and (labelMismatches == 0);

        REQUIRE(success == true);
    }

}


// Here should be another test that checks:
// For each cluster point: is there another cluster point higher / deeper in the tree for a given iteration of
// generating cluster points?
// Or: Can a cluster of one run be a subset of another cluster within the same run of generating cluster points?
// This may be done by iterating through each cluster point and the corresponding twin trees, though the take will
// likely take ages to compute, whereas this property (whether it hold or not) may have important semantic implication.
// From what I understand from the LCA Method to generate Cluster Points this may hold true, and beyond that we'd not
// even need to recurse because it generates all possible cluster points in one iteration.