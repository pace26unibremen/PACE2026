#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Remove Edge", "[Forest, removeEdge]")
{
    SECTION("Remove edge from root node")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  1,  2); // [0] 0 +-- 1 (label 11)
        nodes->emplace_back( 0,  2, -1, -1); // [1]   |
        nodes->emplace_back( 0,  1, -1, -1); // [2]   +-- 2 (label 22)
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(1, 11);
        terminals->emplace(2, 22);
        auto roots = make_shared<vector<int>>();
        roots->emplace_back(0);
        auto tree = Forest(nodes, terminals, roots);

        tree.removeEdge(1);
        REQUIRE(tree.isValid());
        REQUIRE(tree.RootIndices().size() == 2);
        REQUIRE(tree.RootIndices()[0] == 1);
        REQUIRE(tree.RootIndices()[1] == 2);
    }

    SECTION("Reduce non terminal leaf node")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  1,  2); // [0]  0 +-- 1 +-- 3 (label 11)
        nodes->emplace_back( 0,  2,  3,  4); // [1]    |     |
        nodes->emplace_back( 0,  1, -1, -1); // [2]    |     +-- 4 (label 22)
        nodes->emplace_back( 1,  4, -1, -1); // [3]    |
        nodes->emplace_back( 1,  3, -1, -1); // [4]    +-- 2 (label 33)

        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(3, 11);
        terminals->emplace(4, 22);
        terminals->emplace(2, 33);

        auto roots = make_shared<vector<int>>();
        roots->emplace_back(0);

        auto tree = Forest(nodes, terminals, roots);

        tree.removeEdge(3);
        REQUIRE(tree.isValid());
        REQUIRE(tree.RootIndices().size() == 2);
        REQUIRE(tree.RootIndices()[0] == 0);
        REQUIRE(tree.RootIndices()[1] == 3);
    }
}
