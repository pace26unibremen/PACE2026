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

TEST_CASE("Equality of Label Sets for a given Cluster Point")
{
    SECTION("Check for equality of two simple trees - 'equal instance'")
    {

        bool labelMismatches = 0;





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
                    if (!referenceSet.contains(label))
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