//
// Created by kaufm on 05.03.2026.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/Graph/Forest.hpp"
#include "../src/Graph/Instance.hpp"
#include "../src/Solver/BranchingSolver.hpp"
#include "../src/Solver/Rule/ThreeTwoChainReductionRule.hpp"

using namespace graph;
using namespace std;
using namespace solver;

TEST_CASE("Reduce 3-2-Chain - Tree 1", "[Forest, ThreeTwoChainReductionRule, AbstractAction]")
{
    std::string TEST_EXAMPLES_DIR = "D:/Study_Work/PACEMasterprojekt/childRename/pace2026/res/tests";
    std::string TINY_EXAMPLES_DIR = "D:/Study_Work/PACEMasterprojekt/childRename/pace2026/res/tiny";
    SECTION("Section 1")
    {
        auto tree1 = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_for_3_2_chain_reduction_test.tree");
        REQUIRE(tree1 != nullptr);
        auto rule = ThreeTwoChainReductionRule::isApplicable(tree1,std::make_shared<Context>());
        REQUIRE(rule);
        rule->apply();
        auto tree1RuleApplied = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_for_3_2_chain_reduction_test_2.tree");
        //REQUIRE(tree1 == tree1RuleApplied);
    }
    // SECTION("Section 2")
    // {
    //     auto tree1 = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_for_3_2_chain_reduction_long_test");
    //     auto tree1Applied = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_for_3_2_chain_reduction_long_test_2");
    //     auto rule = ThreeTwoChainReductionRule::isApplicable(tree1,std::make_shared<Context>());
    //     REQUIRE(rule);
    //     rule->apply();
    //     REQUIRE(tree1Applied == tree1);
    //
    // }
    SECTION("Section 3")
    {
        auto tree1 = ReadInstance(std::string(TINY_EXAMPLES_DIR) + "tiny04.nw");
        auto rule = ThreeTwoChainReductionRule::isApplicable(tree1,std::make_shared<Context>());
        REQUIRE(rule);
        rule->apply();
        auto tree2 = ReadInstance(std::string(TINY_EXAMPLES_DIR) + "tiny04.nw");
        REQUIRE_FALSE(tree1 == tree2);
    }

}