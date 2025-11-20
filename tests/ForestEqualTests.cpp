#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Equality of Trees", "[Forest, equal, ==]")
{
    SECTION("Check for equality of two simple trees - 'equal instance'")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple_reordered.tree");

        REQUIRE(t1 == t2);
        REQUIRE(t2 == t1);
        REQUIRE(t1 == t1);
    }

    SECTION("Check for equality of two simple trees - 'equal instance'")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree");
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_2_2_singleVertexTrees.tree", 0, 2);

        REQUIRE_FALSE(t1 == f2);
        REQUIRE_FALSE(f2 == t1);
    }

    SECTION("Check for equality of two trees - 'equal instance'")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1_reordered.tree");

        REQUIRE(t1 == t2);
        REQUIRE(t2 == t1);
        REQUIRE(t1 == t1);
    }

    SECTION("Check for equality of two trees - 'equal instance'")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example2.tree");

        REQUIRE_FALSE(t1 == t2);
        REQUIRE_FALSE(t2 == t1);
    }
}

TEST_CASE("Equality of Forests", "[Forest, equal, ==]")
{
    SECTION("Check for equality of two forests - 'equal instance'")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_4_6_simple.tree",0,4);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_4_6_simple_reordered.tree",0,4);

        REQUIRE(f1 == f2);
        REQUIRE(f2 == f1);
        REQUIRE(f1 == f1);
    }

    SECTION("Check for equality of two forests - 'unequal instance'")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_4_6_simple.tree",0,4);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_6_6_singleVertexTrees.tree", 0, 6);

        REQUIRE_FALSE(f1 == f2);
        REQUIRE_FALSE(f2 == f1);
    }
}
