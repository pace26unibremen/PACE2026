#include "Solver/BranchingSolver.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace graph;

int main(int, char**)
{
    // auto instance = ReadInstance("../../../pace2026_Instances/2_25/instances/1.nw");
    auto instance = ReadInstance(string(RES_DIR) + "examples/1.nw");

    const auto config = std::make_shared<solver::BranchingSolverConfiguration>();
    // config->debPlugin = std::make_shared<solver::DebugPlugin>("../../debug");

    config->activeRules = {
            solver::CutBranchRule::isApplicable,
            solver::EqualForestsRule::isApplicable,
            solver::SingleVertexTreePropagationRule::isApplicable,
            solver::PairUnconnectedBranchingRule::isApplicable,
            solver::PairEqualRule::isApplicable,
            solver::PairPathBranchingRule::isApplicable,
            solver::DebugAssertFalseRule::isApplicable};

    auto solver = solver::BranchingSolver(instance, config);

    const auto t0 = std::clock();
    auto solved = solver.solve();
    const auto t1 = std::clock();

    const auto t_delta_ms = ((double) (t1 - t0)) / ((double) CLOCKS_PER_SEC / 1000.0);

    if (solved)
    {
        solver.unapplyReductions();
        graph::WriteInstance(solver.Instance(), cout);
        cout << "solved in: " << t_delta_ms << " ms" << endl;
    }
    else
    {
        cout << "doesnt solved completely:" << t_delta_ms << " ms" << endl;
    }
    return 17;
}
