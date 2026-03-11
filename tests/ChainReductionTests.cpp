#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"
#include "../src/Graph/Instance.hpp"
#include "../src/Solver/Action/DeleteNodeActionInChains.h"
#include "../src/Solver/BranchingSolver.hpp"

using namespace graph;
using namespace std;
using namespace solver;

TEST_CASE("Reduce Chain - Tree 1", "[Forest, DeleteNodeActionInChains, AbstractAction]")
{
    SECTION("Section 1")
    {
        auto i = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        auto rule = ChainReductionRule::isApplicable(i,std::make_shared<Context>());
        REQUIRE(rule);
        rule->apply();
    }

    SECTION("Section 2")
    {
        auto i = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        auto solver = BranchingSolver(i);
        solver.solve();

    }
}