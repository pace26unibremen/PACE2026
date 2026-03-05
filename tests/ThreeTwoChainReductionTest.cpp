//
// Created by kaufm on 05.03.2026.
//

#include <catch2/catch_test_macros.hpp>
#include "../cmake-build-debug/_deps/catch2-src/src/catch2/catch_test_macros.hpp"
#include "../src/Graph/Forest.hpp"
#include "../src/Graph/Instance.hpp"
#include "../src/Solver/BranchingSolver.hpp"
#include "../src/Solver/Rule/ThreeTwoChainReductionRule.hpp"

using namespace graph;
using namespace std;
using namespace solver;

TEST_CASE("Reduce 3-2-Chain - Tree 1", "[Forest, ThreeTwoChainReductionRule, AbstractAction]")
{
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

}