#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Instance.hpp"
#include "../src/Solver/BranchingSolver.hpp"

#include <chrono>

namespace
{
const std::unordered_map<std::string, unsigned int> optimalSize = {
    {"tiny01.nw", 4},
    {"tiny02.nw", 1},
    {"tiny05.nw", 3},
    {"tiny06.nw", 3},
};

std::shared_ptr<solver::BranchingSolver> makeSolver(const std::string& f)
{
    auto config = std::make_shared<solver::BranchingSolverConfiguration>();
    config->boundedDephtSearch = false;
    auto instance = graph::ReadInstance(std::string(RES_DIR) + "tiny/" + f);
    return std::make_shared<solver::BranchingSolver>(instance, config);
}
}  // namespace

TEST_CASE("A deadline far in the future does not change the optimal solution", "[BranchingSolver, Deadline]")
{
    for (const auto& [f, opt] : optimalSize)
    {
        SECTION("solve " + f)
        {
            auto solver = makeSolver(f);
            solver->setDeadline(std::chrono::steady_clock::now() + std::chrono::hours(1));

            CHECK(solver->solve());
            solver->unapplyReductions();
            CHECK(solver->Instance()->at(0)->Roots().size() == opt);
        }
    }
}

TEST_CASE("A deadline that has already passed still yields a valid forest", "[BranchingSolver, Deadline]")
{
    for (const auto& [f, opt] : optimalSize)
    {
        SECTION("solve " + f)
        {
            auto solver = makeSolver(f);
            // Already in the past: the solver must still search until it has a first
            // candidate, then stop gracefully and return a valid (upper-bound) forest.
            solver->setDeadline(std::chrono::steady_clock::now() - std::chrono::seconds(1));

            CHECK(solver->solve());
            solver->unapplyReductions();
            CHECK(solver->Instance()->at(0)->isValid());
            CHECK(solver->Instance()->at(0)->Roots().size() >= opt);
        }
    }
}
