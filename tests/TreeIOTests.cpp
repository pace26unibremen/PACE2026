#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <fstream>

#include "../src/Graph/Tree.hpp"

TEST_CASE("Read and Write", "[Tree, TreeIO]")
{
    SECTION("Read")
    {
        auto t1 = graph::Tree(std::string(TEST_EXAMPLES_DIR) + "test1.tree");
        auto t2 = graph::Tree(std::string(TEST_EXAMPLES_DIR) + "test2.tree");

        INFO("compare number of nodes");
        REQUIRE(t1.Nodes().size() == 11);
        REQUIRE(t2.Nodes().size() == 11);
        INFO("check validity of tree structure");
        REQUIRE(t1.isValid());
        REQUIRE(t2.isValid());
    }


    SECTION("Write")
    {
        auto t1 = graph::Tree(std::string(TEST_EXAMPLES_DIR) + "test1.tree");
        auto t2 = graph::Tree(std::string(TEST_EXAMPLES_DIR) + "test2.tree");

        std::ifstream t1File(std::string(TEST_EXAMPLES_DIR) + "test1.tree");
        std::ifstream t2File(std::string(TEST_EXAMPLES_DIR) + "test2.tree");

        std::string t1Original, t2Original;

        getline(t1File, t1Original);
        getline(t2File, t2Original);

        std::stringstream t1Write, t2Write;
        t1.write(t1Write);
        t2.write(t2Write);

        INFO("compare newick representation before and after write");
        REQUIRE(t1Write.str() == t1Original);
        REQUIRE(t2Write.str() == t2Original);
    }
}
