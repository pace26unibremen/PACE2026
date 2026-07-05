#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../src/Solver/Cluster/ClusterBudget.hpp"

#include <chrono>
#include <vector>

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

TEST_CASE("ClusterBudget weights each cluster's share by its size", "[ClusterBudget]")
{
    const clock::time_point start{seconds(0)};

    SECTION("a cluster's share is proportional to its weight")
    {
        // weights {1,3} -> total weight 4; first cluster gets 1/4 of 100s
        ClusterBudget budget(start, 100.0, std::vector<double>{1.0, 3.0});
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(25.0));
    }

    SECTION("a larger cluster gets a proportionally larger share")
    {
        // weights {3,1} -> first (bigger) cluster gets 3/4 of 100s
        ClusterBudget budget(start, 100.0, std::vector<double>{3.0, 1.0});
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(75.0));
    }

    SECTION("the last cluster receives all of the remaining budget regardless of weight")
    {
        ClusterBudget budget(start, 100.0, std::vector<double>{1.0, 3.0});
        const clock::time_point now = start + seconds(25);  // first cluster used its 25s
        // 75s remain, only the last cluster (weight 3) is left -> it gets all 75s
        REQUIRE(secondsUntil(budget.deadlineFor(1, now), now) == Approx(75.0));
    }

    SECTION("time unused by an earlier cluster rolls forward, split by remaining weight")
    {
        // weights {1,1,2}; first cluster's fair share is 100 * 1/4 = 25s
        ClusterBudget budget(start, 100.0, std::vector<double>{1.0, 1.0, 2.0});
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(25.0));
        // first cluster finished early after 5s: 95s remain, split over weights {1,2}
        const clock::time_point now = start + seconds(5);
        REQUIRE(secondsUntil(budget.deadlineFor(1, now), now) == Approx(95.0 / 3.0));
    }

    SECTION("uniform weights reproduce the equal split")
    {
        ClusterBudget weighted(start, 100.0, std::vector<double>{1.0, 1.0, 1.0, 1.0});
        ClusterBudget counted(start, 100.0, 4);
        REQUIRE(secondsUntil(weighted.deadlineFor(0, start), start) ==
                Approx(secondsUntil(counted.deadlineFor(0, start), start)));
    }

    SECTION("degenerate all-zero weights fall back to an equal split by count")
    {
        ClusterBudget budget(start, 100.0, std::vector<double>{0.0, 0.0, 0.0, 0.0});
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(25.0));
    }

    SECTION("an empty weight vector is treated as a single cluster")
    {
        ClusterBudget budget(start, 100.0, std::vector<double>{});
        REQUIRE(secondsUntil(budget.deadlineFor(0, start), start) == Approx(100.0));
    }
}
