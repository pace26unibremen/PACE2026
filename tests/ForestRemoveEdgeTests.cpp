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

    SECTION("Remove several edges")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);

        f1.removeEdge(2);
        f1.removeEdge(7);
        f1.removeEdge(10);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 4);
        REQUIRE(f1.RootIndices()[0] == 2);
        REQUIRE(f1.RootIndices()[1] == 0);
        REQUIRE(f1.RootIndices()[2] == 7);
        REQUIRE(f1.RootIndices()[3] == 10);

        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_4_6_simple.tree",0,4);
        REQUIRE(f1 == f2);
    }
}

TEST_CASE("Remove Edge - sibling & root order", "[Forest, removeEdge]")
{
    SECTION("Case 1")
    {
        INFO("Remove Edge to left root child");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        f1.removeEdge(4);
        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 4);
        REQUIRE(f1.RootIndices()[0] == 0);
        REQUIRE(f1.RootIndices()[1] == 4);
        REQUIRE(f1.RootIndices()[2] == 18);
        REQUIRE(f1.RootIndices()[3] == 11);
    }

    SECTION("Case 2")
    {
        INFO("Remove Edge to right root child");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        f1.removeEdge(11);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 4);
        REQUIRE(f1.RootIndices()[0] == 0);
        REQUIRE(f1.RootIndices()[1] == 4);
        REQUIRE(f1.RootIndices()[2] == 18);
        REQUIRE(f1.RootIndices()[3] == 11);
    }


    SECTION("Case 3")
    {
        INFO("Remove Edge to inner node with smallest terminal");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        f1.removeEdge(6);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 4);
        REQUIRE(f1.RootIndices()[0] == 0);
        REQUIRE(f1.RootIndices()[1] == 6);
        REQUIRE(f1.RootIndices()[2] == 18);
        REQUIRE(f1.RootIndices()[3] == 3);

        auto r = f1.Nodes()[3];
        REQUIRE(r.firstChildIndex == 11);
        REQUIRE(r.secondChildIndex == 4);
        auto p = f1.Nodes()[4];
        REQUIRE(p.firstChildIndex == 8);
        REQUIRE(p.secondChildIndex == 7);
    }

    SECTION("Case 4")
    {
        INFO("Remove Edge to inner node with high terminals");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        f1.removeEdge(8);

        REQUIRE(f1.isValid());
        REQUIRE(f1.RootIndices().size() == 4);
        REQUIRE(f1.RootIndices()[0] == 0);
        REQUIRE(f1.RootIndices()[1] == 3);
        REQUIRE(f1.RootIndices()[2] == 18);
        REQUIRE(f1.RootIndices()[3] == 8);

        auto r = f1.Nodes()[3];
        REQUIRE(r.firstChildIndex == 5);
        REQUIRE(r.secondChildIndex == 11);
    }

}

