#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"
#include "../src/Graph/ForestIO.hpp"
#include "../src/Graph/Instance.hpp"
#include "../src/Solver/BranchingSolver.hpp"

#include <iostream>

using namespace graph;
using namespace std;
using namespace solver;

TEST_CASE("Reduce Chain - Tree 1", "[Forest, DeleteNodeActionInChains, AbstractAction]")
{
    SECTION("Section 1")
    {
        auto i = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        auto rule = ChainReductionRule::isApplicable(i,std::make_shared<Context>());
        INFO("Apply ChainReductionRule on Tree");
        REQUIRE(rule);
        rule->apply();
        REQUIRE(rule->IsApplied());

        // graph::Forest forest = {nullptr,nullptr,nullptr,nullptr};
        // for (auto current : *i)
        // {
        //    forest = current->copy();
        //
        // }
    }

    SECTION("Section 2")
    {
        auto i = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        for(auto forest : *i)
        {
            forest->write(cout);
        }
        INFO("Section 2 - Check for Changes after apply");
        auto rule = ChainReductionRule::isApplicable(i,std::make_shared<Context>());
        REQUIRE(rule);
        rule->apply();
        REQUIRE(rule->IsApplied());
        for(auto forest : *i)
        {
            forest->write(cout);
        }
    }

    SECTION("Section 3 - Overall Test")
    {
        auto i = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        // for(auto forest : *i)
        // {
        //     forest->write(cout);
        // }
        auto j = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        INFO("Test the Functionally within Branching Solver");
        auto solver = BranchingSolver(i);
        auto e = solver.solve();
        for (auto forest : *i)
        {
            REQUIRE(e != forest);
        }
        // e->write(cout);
    }
}