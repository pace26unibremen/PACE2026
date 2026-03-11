//
// Created by user on 3/10/26.
//
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Instance.hpp"


#include "../src/Graph/InteriorTwinRelation.hpp"
#include "../src/Graph/ClusterPointGenerator.hpp"
#include "../src/Graph/Forest.hpp"
#include "functional"
#include "iostream"
// Let R be the set of labels/leafs from a given cluster point from the list of cluster points (which belong to the same forest).  (Reference).
// Let C be the set of a twin of the cluster point (Comparator).
// Test Question: For each x in C : If x in C then x in R AND Size of C = Size of R. (If not, fail test.)
TEST_CASE("ClusterLabelEquivalence")
{
    SECTION("Check if the labels of a cluster point are the same across all clusters.")
    {
        int labelMismatches = 0;





        std::shared_ptr<graph::Instance>  instance = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_100_11_stressTest.tree");

        graph::InteriorTwinRelation interiorTwins = graph::InteriorTwinRelation(instance);
        graph::ClusterPointGenerator generator = graph::ClusterPointGenerator(instance, &interiorTwins);
        std::vector<graph::Node*> clusterPoints = generator.clusterPoints;



        std::function<void(std::set<unsigned int>*, graph::Node*, graph::Forest* forest)> fetchLabels =
            [&](std::set<unsigned int>* set, graph::Node* node, graph::Forest* forest) -> void {

            if (node->leftChild == nullptr && node->rightChild == nullptr)
                set->insert(forest->TerminalToLabel().at(node));

            if (node->leftChild) fetchLabels(set, node->leftChild, forest);
            if (node->rightChild) fetchLabels(set, node->rightChild, forest);
        };


        std::function<std::shared_ptr<graph::Forest>(graph::Node*)> getForestOfNode = [&](graph::Node* node) -> std::shared_ptr<graph::Forest>{
            graph::Node* buffer = node;
            while (buffer->parent != nullptr) buffer = buffer->parent;

            for (const auto& forest : *instance)
            {
                if (forest->Roots().front() == buffer) return forest;
            }
            return nullptr;

        };



        for (auto clusterPoint : clusterPoints)
        {
            std::set<unsigned int> referenceSet = std::set<unsigned int>();



            fetchLabels(&referenceSet, clusterPoint, getForestOfNode(clusterPoint).get());


            for (const auto& twin : interiorTwins.nodeToTwins[clusterPoint])
            {
                std::set<unsigned int> comparisonSet = std::set<unsigned int>();

                fetchLabels(&comparisonSet, twin, getForestOfNode(twin).get());

                for (const auto& label : comparisonSet)
                {
                    if (!referenceSet.contains(label) || referenceSet.size() != comparisonSet.size())
                    {
                        labelMismatches += 1;
                    }
                }


            }


            referenceSet.clear();

        }




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
        std::vector<graph::Node*> clusterPoints = generator.clusterPoints;
        
        std::set<graph::Node*> testSet = std::set<graph::Node*>();
        for (auto forest : *instance){

            auto root = forest->Roots().front();
            for (const auto& twinOfRoot : interiorTwins.nodeToTwins[root])
            {
                testSet.insert(twinOfRoot);


                for (auto twinOfTwin : interiorTwins.nodeToTwins[twinOfRoot])
                {
                    testSet.insert(twinOfTwin);
                }

            }


        }

        // This counts the number of "foreign nodes" in the root twin test set.
        int testSetContaminator = 0;
        for (auto forest : *instance)
        {
            auto root = forest->Roots().front();
            if (!testSet.contains(root))
                testSetContaminator += 1;
        }

        // This is a bit hacky but should suffice for the test...
        REQUIRE(testSet.size()+testSetContaminator == instance->size());
    }


}