#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Remove Edge - Simple Tree with two Terminals", "[Forest, removeEdge]")
{
    SECTION("Remove edge left child")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree",0,1);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_2_2_singleVertexTrees.tree",0,2);

        f1.removeEdge(1);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 2);
        REQUIRE(f1.RootIndices()[0] == 1);
        REQUIRE(f1.RootIndices()[1] == 2);
        REQUIRE(f1 == f2);
    }

    SECTION("Remove edge from right child")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree",0,1);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_2_2_singleVertexTrees.tree",0,2);

        f1.removeEdge(2);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 2);
        REQUIRE(f1.RootIndices()[0] == 1);
        REQUIRE(f1.RootIndices()[1] == 2);
        REQUIRE(f1 == f2);
    }
}

TEST_CASE("Remove Edge - Tree with six Terminals", "[Forest, removeEdge]")
{
    SECTION("Remove inner edge 1")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);

        f1.removeEdge(5);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 2);
        REQUIRE(f1.RootIndices()[0] == 0);
        REQUIRE(f1.RootIndices()[1] == 5);
    }

    SECTION("Remove inner edge 2")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);

        f1.removeEdge(2);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 2);
        REQUIRE(f1.RootIndices()[0] == 2);
        REQUIRE(f1.RootIndices()[1] == 0);
    }
}
