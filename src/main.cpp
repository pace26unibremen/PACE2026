#include "Graph/Forest.hpp"

#include <iostream>
#include <string>
#include "Solver/BranchingSolver.hpp"
#include <fstream>

using namespace std;
using namespace graph;

int main(int, char**)
{
        auto f = std::ofstream("../../stats.csv");
        f << "id,time\n;";

        for (auto i : {"01","02","03"} )
        {
            auto instance = ReadInstance(string(RES_DIR) + "tiny/tiny"+i+ ".nw");
            auto solver = solver::BranchingSolver(instance);
            solver.ActiveRules() = {
                solver::CutBranchRule::isApplicable,
                solver::EqualForestsRule::isApplicable,
                solver::SingleVertexTreePropagationRule::isApplicable,
                solver::PairUnconnectedBranchingRule::isApplicable,
                solver::PairEqualRule::isApplicable,
                solver::PairPathBranchingRule::isApplicable,
                solver::DebugAssertFalseRule::isApplicable};

            auto t0 = std::clock();
            auto solution = solver.solve();
            auto t1 = std::clock();

            auto t_delta_ms = ((double) (t1 - t0)) / ((double) CLOCKS_PER_SEC / 1000.0);

            solution->write(cout);

            f << i << "," << t_delta_ms << endl;
        }

        return 17;

}
