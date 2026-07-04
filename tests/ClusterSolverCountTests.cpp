#include <catch2/catch_test_macros.hpp>

#include "../src/Graph/Instance.hpp"
#include "../src/Solver/Cluster/ClusterRange.hpp"
#include "../src/Solver/Cluster/ClusterSolver.hpp"

TEST_CASE("ClusterSolver reports the number of clusters it produced", "[ClusterSolver]")
{
    for (const std::string& f : {"tiny01.nw", "tiny05.nw", "tiny07.nw"})
    {
        SECTION("count clusters of " + f)
        {
            auto instance = graph::ReadInstance(std::string(RES_DIR) + "tiny/" + f);
            solver::ClusterSolver cs(instance);
            cs.solve();

            // There is always at least one cluster (a dummy wrapping the whole instance
            // when no cluster points exist), and the reported count must match what the
            // cluster range actually iterates over.
            unsigned int iterated = 0;
            for ([[maybe_unused]] const auto& entry : cs.Clusters())
            {
                ++iterated;
            }

            CHECK(cs.clusterCount() >= 1);
            CHECK(cs.clusterCount() == iterated);
        }
    }
}
