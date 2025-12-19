#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Sub Forest for simple trees with two terminals", "[Forest, sub forest, <=]")
{
    SECTION("Check sub forest on two simple equal trees")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple_reordered.tree");

        REQUIRE(t1.isTrueSubtreeOf(t2));
        REQUIRE(t2.isTrueSubtreeOf(t1));
        REQUIRE(t1.isTrueSubtreeOf(t1));
    }

    SECTION("Check sub forest on two simple trees")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree");
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_2_2_singleVertexTrees.tree", 0, 2);

        REQUIRE(f2.isTrueSubtreeOf(t1));
        REQUIRE_FALSE(t1.isTrueSubtreeOf(f2));
    }
}


TEST_CASE("Sub Forest", "[Forest, sub forest, <=]")
{
    SECTION("Check sub forest - example 1")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_4_6_simple2.tree", 0, 4);

        REQUIRE(f2.isTrueSubtreeOf(t1));
        REQUIRE_FALSE(t1.isTrueSubtreeOf(f2));
    }

    SECTION("Check sub forest - example 2")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example2.tree");
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_6_simple.tree", 0, 3);

        REQUIRE(f2.isTrueSubtreeOf(t1));
        REQUIRE_FALSE(t1.isTrueSubtreeOf(f2));
    }
}

TEST_CASE("Identical Subforest", "[Forest, subforest]")
{
    SECTION("Check identical subforest - example 1")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");

        REQUIRE(t1.hasIdenticalSubtree(t2, t1.RootIndices().at(0), t2.RootIndices().at(0)));
    }

    SECTION("Check identical subforest - example 2")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree");
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree");

        for (int i = 0; i<f1.RootIndices().size(); i++)
        {
            REQUIRE(f1.hasIdenticalSubtree(f2, f1.RootIndices().at(i), f2.RootIndices().at(i)));
        }
    }
}