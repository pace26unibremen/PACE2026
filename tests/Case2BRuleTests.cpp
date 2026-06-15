#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"
#include "../src/Graph/Instance.hpp"
#include "../src/Solver/BranchingSolver.hpp"
#include "../src/Solver/Rule/Case2BRule.hpp"
#include "catch2/catch_config.hpp"

#include <algorithm>

using namespace graph;
using namespace std;
using namespace solver;

TEST_CASE("Case2BRule tests", "[Forest, Case2BRule]")
{
    SECTION("Two Trees, applicable")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example5.tree",6,1);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example2.tree",6, 1);

        auto instance = std::make_shared<std::vector<std::shared_ptr<Forest>>>();
        instance->push_back(std::make_shared<Forest>(f1));
        instance->push_back(std::make_shared<Forest>(f2));

        auto rule = Case2BRule::isApplicable(instance, nullptr);

        REQUIRE(rule != nullptr);

        INFO("Apply Rule");
        rule->apply();

        REQUIRE(f1.Roots().size() == 3);
        REQUIRE(f2.Roots().size() == 3);

        auto end1 = f1.Roots().end();
        Node* cut11 = f1.LabelToTerminal()[2];
        Node* cut12 = f1.LabelToTerminal()[3];
        REQUIRE(std::find(f1.Roots().begin(), end1, cut11) != end1);
        REQUIRE(std::find(f1.Roots().begin(), end1, cut12) != end1);

        auto end2 = f2.Roots().end();
        Node* cut21 = f2.LabelToTerminal()[2];
        Node* cut22 = f2.LabelToTerminal()[3];
        REQUIRE(std::find(f2.Roots().begin(), end2, cut21) != end2);
        REQUIRE(std::find(f2.Roots().begin(), end2, cut22) != end2);

        INFO("Undo Action");
        rule->unapply();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example5.tree",6,1);
        auto f2Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example2.tree",6, 1);
        REQUIRE(f1 == f1Copy);
        REQUIRE(f2 == f2Copy);
    }

    SECTION("Two Trees, not applicable")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example2.tree",6,1);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example5.tree",6, 1);

        auto instance = std::make_shared<std::vector<std::shared_ptr<Forest>>>();
        instance->push_back(std::make_shared<Forest>(f1));
        instance->push_back(std::make_shared<Forest>(f2));

        auto rule = Case2BRule::isApplicable(instance, nullptr);

        REQUIRE(rule == nullptr);
    }
}

TEST_CASE("Case2BRule tests with instance 10ca71f1637bc1d9a45e0966f60863d2", "[Forest, Case2BRule]")
{
    SECTION("Three Trees")
    {
        auto config = std::make_shared<solver::BranchingSolverConfiguration>();
        config->boundedDephtSearch = false;
        auto instance = graph::ReadInstance(std::string(RES_DIR) + "tests/instance_3_11.nw");
        auto solver = solver::BranchingSolver(instance, config);
        auto solved = solver.solve();

        solver.unapplyReductions();

        auto rule = Case2BRule::isApplicable(instance, nullptr);
    }
}