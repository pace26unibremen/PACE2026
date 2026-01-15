#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Maximum common X-Forest of Trees", "[Forest, subforest]")
{
    SECTION("Check for reordering of the same tree")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1_reordered.tree");

        REQUIRE(t1.maximumCommonSubforestRoots((t2)) == t1.RootIndices());
    }

    SECTION("Check for single vertex subtrees")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree");
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_6_6_singleVertexTrees.tree");

        REQUIRE(t1.maximumCommonSubforestRoots(f2).size() == 6);
    }
}