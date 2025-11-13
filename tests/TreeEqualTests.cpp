#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"

using namespace graph;
using namespace std;

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
        auto roots_1 = make_shared<vector<int>>();
        roots_1->emplace_back(0);
        auto tree_1 = Forest(nodes_1, terminals_1, roots_1);

        auto nodes_2 = make_shared<vector<Node>>();
        nodes_2->emplace_back(-1, -1, -1, -1); // [0]
        nodes_2->emplace_back(-1, -1,  3,  2); // [1]  1 +-- 3 (222)
        nodes_2->emplace_back( 1,  3, -1, -1); // [2]    |
        nodes_2->emplace_back( 1,  2, -1, -1); // [3]    +-- 2 (111)
        auto terminals_2 = make_shared<unordered_map<int, unsigned int>>();
        terminals_2->emplace(3, 222);
        terminals_2->emplace(2, 111);
        auto roots_2 = make_shared<vector<int>>();
        roots_2->emplace_back(1);
        auto tree_2 = Forest(nodes_2, terminals_2, roots_2);

        REQUIRE(tree_1 == tree_2);
    }
}
