#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../src/Solver/Cluster/ClusterBudget.hpp"

#include <chrono>

using Catch::Approx;
using solver::ClusterBudget;

namespace
{
using clock = std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::seconds;

double secondsUntil(clock::time_point deadline, clock::time_point now)
{
    return duration_cast<duration<double>>(deadline - now).count();
}
}  // namespace

TEST_CASE("ClusterBudget splits the remaining budget across remaining clusters", "[ClusterBudget]")
{
    const clock::time_point start{seconds(0)};

    SECTION("the first of four clusters gets an equal quarter of the total budget")
    {
        ClusterBudget budget(start, 100.0, 4);
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(25.0));
    }

    SECTION("time left unused by earlier clusters rolls forward to the next one")
    {
        ClusterBudget budget(start, 100.0, 4);
        const clock::time_point now = start + seconds(10);  // first cluster finished early
        // 90s remain, split across the 3 clusters not yet started -> 30s each
        REQUIRE(secondsUntil(budget.deadlineFor(1, now), now) == Approx(30.0));
    }

    SECTION("the last cluster receives all of the remaining budget")
    {
        ClusterBudget budget(start, 100.0, 4);
        const clock::time_point now = start + seconds(70);
        REQUIRE(secondsUntil(budget.deadlineFor(3, now), now) == Approx(30.0));
    }

    SECTION("once the total budget is spent the deadline is immediate")
    {
        ClusterBudget budget(start, 100.0, 4);
        const clock::time_point now = start + seconds(120);
        REQUIRE(budget.deadlineFor(2, now) == now);
    }

    SECTION("a single cluster is granted the whole budget")
    {
        ClusterBudget budget(start, 100.0, 1);
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(100.0));
    }
}
