#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Forest.hpp"
#include "../src/Graph/ForestIO.hpp"

#include <fstream>
#include <sstream>

TEST_CASE("Read and Write (single tree forest)", "[Forest, ForestIO]")
{
    SECTION("Read")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree1.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree2.tree");

        INFO("compare number of nodes");
        REQUIRE(t1.Nodes().size() == 11);
        REQUIRE(t2.Nodes().size() == 11);
        INFO("Check number of trees");
        REQUIRE(t1.RootIndices().size() == 1);
        REQUIRE(t2.RootIndices().size() == 1);
        INFO("check validity of tree structure");
        REQUIRE(t1.isValid());
        REQUIRE(t2.isValid());
    }

    SECTION("Write")
    {
        auto t1 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree1.tree");
        auto t2 = graph::Forest(std::string(TEST_EXAMPLES_DIR) + "tree2.tree");

        std::ifstream t1StreamOriginal(std::string(TEST_EXAMPLES_DIR) + "tree1.tree");
        std::ifstream t2StreamOriginal(std::string(TEST_EXAMPLES_DIR) + "tree2.tree");
        std::stringstream t1StreamWrite, t2StreamWrite;
        t1.write(t1StreamWrite);
        t2.write(t2StreamWrite);

        std::string t1Original, t2Original;
        std::string t1Write,  t2Write;

        INFO("compare newick representation before and after write");
        getline(t1StreamOriginal, t1Original);
        getline(t1StreamWrite, t1Write);
        REQUIRE(t1Write == t1Original);

        getline(t2StreamOriginal, t2Original);
        getline(t2StreamWrite, t2Write);
        REQUIRE(t2Write == t2Original);

        // the streams should contain a remaining empty line
        getline(t1StreamOriginal, t1Original);
        getline(t1StreamWrite, t1Write);
        REQUIRE(t1Write == t1Original);

        getline(t2StreamOriginal, t2Original);
        getline(t2StreamWrite, t2Write);
        REQUIRE(t2Write == t2Original);
    }
}

TEST_CASE("Read and Write (multiple tree forest)", "[Forest, ForestIO]")
{
    auto file = std::ifstream(std::string(TEST_EXAMPLES_DIR) + "forest1.tree");
    auto f = graph::ForestIO::ReadNewick(file,0,2);

    SECTION("Read")
    {
        INFO("compare number of nodes");
        REQUIRE(f.Nodes().size() == 22);
        INFO("Check number of trees and root indices");
        REQUIRE(f.RootIndices().size() == 2);
        REQUIRE(f.RootIndices()[0] == 0);
        REQUIRE(f.RootIndices()[1] == 11);
        INFO("check validity of tree structure");
        REQUIRE(f.isValid());
    }

    SECTION("Write")
    {
        auto streamOriginal = std::ifstream(std::string(TEST_EXAMPLES_DIR) + "forest1.tree");
        std::stringstream streamWrite;
        f.write(streamWrite);

        std::string original;
        std::string write;

        INFO("compare newick representation before and after write");

        INFO("comment in original");
        getline(streamOriginal, original);
        REQUIRE(original[0] == '#');

        INFO("First Tree");
        getline(streamOriginal, original);
        getline(streamWrite, write);
        REQUIRE(write == original);

        INFO("comment in original");
        getline(streamOriginal, original);
        REQUIRE(original[0] == '#');

        INFO("compare newick representation before and after write");
        INFO("Second Tree");
        getline(streamOriginal, original);
        getline(streamWrite, write);
        REQUIRE(write == original);

        INFO("compare newick representation before and after write");
        INFO("Empty closing line");
        getline(streamOriginal, original);
        getline(streamWrite, write);
        REQUIRE(write != original);
    }
}
