#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Tree.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Contract nodes", "[Tree, contractNode]")
{
    SECTION("Reduce root node")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1, -1,  1); // [0]    0 --- 1 +-- 2 (101)       1 +-- 2 (101)
        nodes->emplace_back( 0, -1,  2,  3); // [1]            |            ->     |
        nodes->emplace_back( 1,  3, -1, -1); // [2]            +-- 3 (102)         +-- 3 (102)
        nodes->emplace_back( 1,  2, -1, -1); // [3]
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(2, 101);
        terminals->emplace(3, 102);

        auto tree = Tree(nodes, terminals, 0);
        tree.contractNode(0);

        REQUIRE(tree.isValid());
    }

    SECTION("Reduce non terminal leaf node")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  1,  2); // [0]  0 +-- 1 +-- 3 (101)      1 +-- 3 (101)
        nodes->emplace_back( 0,  2,  3,  4); // [1]    |     |            ->    |
        nodes->emplace_back( 0,  1, -1, -1); // [2]    |     +-- 4 (102)        +-- 4 (102)
        nodes->emplace_back( 1,  4, -1, -1); // [3]    |
        nodes->emplace_back( 1,  3, -1, -1); // [4]    +-- 2
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(3, 101);
        terminals->emplace(4, 102);

        auto tree = Tree(nodes, terminals, 0);

        tree.contractNode(2);
        REQUIRE(tree.isValid());
    }

    SECTION("Reduce non terminal leaf node (case 1)")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  1,  5); // [0]  0 +-- 1 --- 2 +-- 3 (101)       0 +-- 2 +-- 3 (101)
        nodes->emplace_back( 0,  5, -1,  2); // [1]    |           |                   |     |
        nodes->emplace_back( 1, -1,  3,  4); // [2]    |           +-- 4 (102)   ->    |     +-- 4 (102)
        nodes->emplace_back( 2,  4, -1, -1); // [3]    |                               |
        nodes->emplace_back( 2,  3, -1, -1); // [4]    +-- 5  (103)                    +-- 5  (103)
        nodes->emplace_back( 0,  1, -1, -1); // [5]
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(3, 101);
        terminals->emplace(4, 102);
        terminals->emplace(5, 103);

        auto tree = Tree(nodes, terminals, 0);

        tree.contractNode(1);

        REQUIRE(tree.isValid());
    }

    SECTION("Reduce non terminal leaf node (case 2)")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  5,  1); // [0]  0 +-- 1 --- 2 +-- 3 (101)       0 +-- 2 +-- 3 (101)
        nodes->emplace_back( 0, -1,  2, -1); // [1]    |           |                   |     |
        nodes->emplace_back( 1,  5,  3,  4); // [2]    |           +-- 4 (102)   ->    |     +-- 4 (102)
        nodes->emplace_back( 2,  4, -1, -1); // [3]    |                               |
        nodes->emplace_back( 2,  3, -1, -1); // [4]    +-------- 5  (103)              +-- 5  (103)
        nodes->emplace_back( 0,  2, -1, -1); // [5]
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(3, 101);
        terminals->emplace(4, 102);
        terminals->emplace(5, 103);

        auto tree = Tree(nodes, terminals, 0);

        tree.contractNode(1);

        REQUIRE(tree.isValid());
    }
}
