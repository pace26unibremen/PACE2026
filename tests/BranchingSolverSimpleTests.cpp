#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "../src/Graph/Instance.hpp"
#include "../src/Solver/BranchingSolver.hpp"

std::unordered_map<std::string, unsigned int> instanceToSolutionSize =
{
    {"tiny01.nw",4},
    {"tiny02.nw",1},
    {"tiny03.nw",7},
    {"tiny04.nw",5},
    {"tiny05.nw",3},
    {"tiny06.nw",3},
    {"tiny07.nw",8},
    {"tiny08.nw",12},
    {"tiny09.nw",5},
    {"tiny10.nw",6},
};


TEST_CASE("BranchingSolver on Tiny Test Set", "[BranchingSolver, Tiny]")
{
    for (const std::string& f : {"tiny01.nw","tiny02.nw","tiny03.nw","tiny04.nw","tiny05.nw",
                             "tiny06.nw","tiny07.nw","tiny08.nw","tiny09.nw","tiny10.nw"})
    {
        SECTION("solve " + f)
        {
            auto instance = graph::ReadInstance(std::string(RES_DIR) + "tiny/" + f);
            auto solver = solver::BranchingSolver(instance);
            auto solution = solver.solve();
            REQUIRE(solution->Roots().size() == instanceToSolutionSize[f]);
        };
    }
}

