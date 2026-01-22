#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"
#include "../src/Solver/Action/DeleteEdgeAction.hpp"

using namespace graph;
using namespace std;
using namespace solver;

TEST_CASE("Delete Edge Action - Simple Tree with two Terminals", "[Forest, DeleteEdgeAction, AbstractAction]")
{
    SECTION("Remove edge left child")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree",0,1);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_2_2_singleVertexTrees.tree",0,2);

        auto action = DeleteEdgeAction(1, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 2);
        REQUIRE(f1.Roots()[0] == 1);
        REQUIRE(f1.Roots()[1] == 2);
        REQUIRE(f1 == f2);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree",0,1);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);

    }

    SECTION("Remove edge from right child")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree",0,1);
        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_2_2_singleVertexTrees.tree",0,2);

        auto action = DeleteEdgeAction(2, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 2);
        REQUIRE(f1.Roots()[0] == 1);
        REQUIRE(f1.Roots()[1] == 2);
        REQUIRE(f1 == f2);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_2_simple.tree",0,1);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }
}

TEST_CASE("Delete Edge Action - Tree with six Terminals", "[Forest, DeleteEdgeAction, AbstractAction]")
{
    SECTION("Remove inner edge 1")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);

        auto action = DeleteEdgeAction(5, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 2);
        REQUIRE(f1.Roots()[0] == 0);
        REQUIRE(f1.Roots()[1] == 5);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }

    SECTION("Remove inner edge 2")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);

        auto action = DeleteEdgeAction(2, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 2);
        REQUIRE(f1.Roots()[0] == 2);
        REQUIRE(f1.Roots()[1] == 0);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }

    SECTION("Remove several edges")
    {
        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);

        auto action1 = DeleteEdgeAction(2, std::make_shared<Forest>(f1));
        auto action2 = DeleteEdgeAction(7, std::make_shared<Forest>(f1));
        auto action3 = DeleteEdgeAction(10, std::make_shared<Forest>(f1));

        INFO("Do Actions");

        action1.doAction();
        action2.doAction();
        action3.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 4);
        REQUIRE(f1.Roots()[0] == 2);
        REQUIRE(f1.Roots()[1] == 0);
        REQUIRE(f1.Roots()[2] == 7);
        REQUIRE(f1.Roots()[3] == 10);

        auto f2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_4_6_simple.tree",0,4);
        REQUIRE(f1 == f2);

        INFO("Undo Actions");
        action3.undoAction();
        action2.undoAction();
        action1.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree_1_6_example1.tree",0,1);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }
}

TEST_CASE("Delete Edge Action - sibling & root order", "[Forest, DeleteEdgeAction, AbstractAction]")
{
    SECTION("Case 1")
    {
        INFO("Remove Edge to left root child");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);
        auto action = DeleteEdgeAction(4, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 4);
        REQUIRE(f1.Roots()[0] == 0);
        REQUIRE(f1.Roots()[1] == 4);
        REQUIRE(f1.Roots()[2] == 18);
        REQUIRE(f1.Roots()[3] == 11);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }

    SECTION("Case 2")
    {
        INFO("Remove Edge to right root child");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        auto action = DeleteEdgeAction(11, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 4);
        REQUIRE(f1.Roots()[0] == 0);
        REQUIRE(f1.Roots()[1] == 4);
        REQUIRE(f1.Roots()[2] == 18);
        REQUIRE(f1.Roots()[3] == 11);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }


    SECTION("Case 3")
    {
        INFO("Remove Edge to inner node with smallest terminal");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        auto action = DeleteEdgeAction(6, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 4);
        REQUIRE(f1.Roots()[0] == 0);
        REQUIRE(f1.Roots()[1] == 6);
        REQUIRE(f1.Roots()[2] == 18);
        REQUIRE(f1.Roots()[3] == 3);

        auto r = f1.Nodes()[3];
        REQUIRE(r.leftChild == 11);
        REQUIRE(r.rightChild == 4);
        auto p = f1.Nodes()[4];
        REQUIRE(p.leftChild == 8);
        REQUIRE(p.rightChild == 7);


        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }

    SECTION("Case 4")
    {
        INFO("Remove Edge to inner node with high terminals");

        auto f1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);

        auto action = DeleteEdgeAction(8, std::make_shared<Forest>(f1));

        INFO("Do Action");
        action.doAction();

        REQUIRE(f1.isValid());
        REQUIRE(f1.Roots().size() == 4);
        REQUIRE(f1.Roots()[0] == 0);
        REQUIRE(f1.Roots()[1] == 3);
        REQUIRE(f1.Roots()[2] == 18);
        REQUIRE(f1.Roots()[3] == 8);

        auto r = f1.Nodes()[3];
        REQUIRE(r.leftChild == 5);
        REQUIRE(r.rightChild == 11);

        INFO("Undo Action");
        action.undoAction();

        auto f1Copy = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "forest_3_12_example3.tree",0,3);
        REQUIRE(f1.isValid());
        REQUIRE(f1 == f1Copy);
    }

}

