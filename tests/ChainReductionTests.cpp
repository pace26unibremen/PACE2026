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
    SECTION("Section 1 - It actually executes")
    {
        auto i = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        auto rule = ChainReductionRule::isApplicable(i,std::make_shared<Context>());
        INFO("Apply ChainReductionRule on Tree");
        REQUIRE(rule);
        rule->apply();
        REQUIRE(rule->IsApplied());
    }

    SECTION("Section 2 - Unapply leads to initial state")
    {
        auto i = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        auto j = graph::ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
        // for(auto forest : *i)
        // {
        //     forest->write(cout);
        // }
        auto rule = ChainReductionRule::isApplicable(i,std::make_shared<Context>());
        REQUIRE(rule);
        rule->apply();
        REQUIRE(rule->IsApplied());
        // for(auto forest : *i)
        // {
        //     forest->write(cout);
        // }
        rule->unapply();
        REQUIRE(not rule->IsApplied());
        REQUIRE(i->size() == j->size());
        for (int k = 0; k < i->size(); k++)
        {
            REQUIRE(i->at(k)->Roots().size() == j->at(k)->Roots().size());
            REQUIRE(i->at(k)->TerminalToLabel().size() == j->at(k)->TerminalToLabel().size());
            REQUIRE(i->at(k)->LabelToTerminal().size() == j->at(k)->LabelToTerminal().size());
            REQUIRE(i->at(k)->Nodes().size() == j->at(k)->Nodes().size());
            for (int n = 0; n < i->at(k)->Nodes().size(); n++)
            {
                REQUIRE(i->at(k)->Nodes().at(n).subtreeTerminals == j->at(k)->Nodes().at(n).subtreeTerminals);
            }
        }

        // for(auto forest : *i)
        // {
        //     forest->write(cout);
        // }
    }

    // SECTION("Section 3 - Overall Test")
    // {
    //     auto i = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
    //     // for(auto forest : *i)
    //     // {
    //     //     forest->write(cout);
    //     // }
    //     auto j = ReadInstance(std::string(TEST_EXAMPLES_DIR) + "forest_2_8_mirrored_chain.tree");
    //     INFO("Test the Functionally within Branching Solver");
    //     auto solver = BranchingSolver(i);
    //     auto e = solver.solve();
    //     for (auto forest : *i)
    //     {
    //         REQUIRE(e != forest);
    //     }
    //     // e->write(cout);
    // }
}