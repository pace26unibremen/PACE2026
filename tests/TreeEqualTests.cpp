#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Tree.hpp"

using namespace graph;
using namespace std;

TEST_CASE("Order Siblings", "[Tree, orderSiblings]")
{
    SECTION("Order siblings - simple")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  1,  2); // [0]  0 +-- 1 (222)      0 +-- 2 (111)
        nodes->emplace_back( 0,  2, -1, -1); // [1]    |            ->    |
        nodes->emplace_back( 0,  1, -1, -1); // [2]    +-- 2 (111)        +-- 1 (222)
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(1, 222);
        terminals->emplace(2, 111);

        auto tree = Tree(nodes, terminals, 0);
        int oldFirstChildIndex = tree.Nodes()[tree.RootIndex()].firstChildIndex;
        int oldSecondChildIndex = tree.Nodes()[tree.RootIndex()].secondChildIndex;
        REQUIRE(tree.Terminals()[oldFirstChildIndex] == 222);
        REQUIRE(tree.Terminals()[oldSecondChildIndex] == 111);

        tree.orderSiblings();

        REQUIRE(tree.isValid());
        int newFirstChildIndex = tree.Nodes()[tree.RootIndex()].firstChildIndex;
        int newSecondChildIndex = tree.Nodes()[tree.RootIndex()].secondChildIndex;
        REQUIRE(tree.Terminals()[newFirstChildIndex] == 111);
        REQUIRE(tree.Terminals()[newSecondChildIndex] == 222);
    }

    SECTION("Order siblings")
    {
        auto nodes = make_shared<vector<Node>>();
        nodes->emplace_back(-1, -1,  1,  2); // [0]  0 +-- 1 +-- 3 (666)
        nodes->emplace_back( 0,  2,  3,  4); // [1]    |     |
        nodes->emplace_back( 0,  1,  5,  6); // [2]    |     +-- 4 (333)
        nodes->emplace_back( 1,  4, -1, -1); // [3]    |
        nodes->emplace_back( 1,  3, -1, -1); // [4]    +-- 2 +-- 5 (999)
        nodes->emplace_back( 2,  6, -1, -1); // [5]          |
        nodes->emplace_back( 2,  5, -1, -1); // [6]          +-- 6 (111)
        auto terminals = make_shared<unordered_map<int, unsigned int>>();
        terminals->emplace(3, 666);
        terminals->emplace(4, 333);
        terminals->emplace(5, 999);
        terminals->emplace(6, 111);

        auto tree = Tree(nodes, terminals, 0);
        tree.orderSiblings();

        REQUIRE(tree.isValid());
        int newFirstSubtreeIndex = tree.Nodes()[tree.RootIndex()].firstChildIndex;
        int newSecondSubtreeIndex = tree.Nodes()[tree.RootIndex()].secondChildIndex;

        int newFirstSubtreeIndex_FirstChild   = tree.Nodes()[newFirstSubtreeIndex].firstChildIndex;
        int newFirstSubtreeIndex_SecondChild  = tree.Nodes()[newFirstSubtreeIndex].secondChildIndex;
        int newSecondSubtreeIndex_FirstChild  = tree.Nodes()[newSecondSubtreeIndex].firstChildIndex;
        int newSecondSubtreeIndex_SecondChild = tree.Nodes()[newSecondSubtreeIndex].secondChildIndex;

        REQUIRE(tree.Terminals()[newFirstSubtreeIndex_FirstChild] == 111);
        REQUIRE(tree.Terminals()[newFirstSubtreeIndex_SecondChild] == 999);
        REQUIRE(tree.Terminals()[newSecondSubtreeIndex_FirstChild] == 333);
        REQUIRE(tree.Terminals()[newSecondSubtreeIndex_SecondChild] == 666);
    }
}

TEST_CASE("Compare Tree ", "[Tree, equal]")
{
    SECTION("Compare Trees - simple")
    {
        auto nodes_1 = make_shared<vector<Node>>();
        nodes_1->emplace_back(-1, -1,  1,  2); // [0]  0 +-- 1 (222)
        nodes_1->emplace_back( 0,  2, -1, -1); // [1]    |
        nodes_1->emplace_back( 0,  1, -1, -1); // [2]    +-- 2 (111)
        auto terminals_1 = make_shared<unordered_map<int, unsigned int>>();
        terminals_1->emplace(1, 222);
        terminals_1->emplace(2, 111);
        auto tree_1 = Tree(nodes_1, terminals_1, 0);

        auto nodes_2 = make_shared<vector<Node>>();
        nodes_2->emplace_back(-1, -1, -1, -1); // [0]
        nodes_2->emplace_back(-1, -1,  3,  2); // [1]  1 +-- 3 (222)
        nodes_2->emplace_back( 1,  3, -1, -1); // [2]    |
        nodes_2->emplace_back( 1,  2, -1, -1); // [3]    +-- 2 (111)
        auto terminals_2 = make_shared<unordered_map<int, unsigned int>>();
        terminals_2->emplace(3, 222);
        terminals_2->emplace(2, 111);
        auto tree_2 = Tree(nodes_2, terminals_2, 1);

        REQUIRE(tree_1 == tree_2);
    }
}
