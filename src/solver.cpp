#include "Solver/BranchingSolver.hpp"
#include <fstream>

int main(int argc, char* argv[]) {
    std::string infile;
    std::string outfile;

    if (argc == 2)
    {
        infile = argv[1];
        outfile = infile + "_solution.tree";
    }
    else if (argc == 3)
    {
        infile = argv[1];
        outfile = argv[2];
    }
    else
    {
        return -1;
    }

    auto t0 = std::clock();
    auto i = graph::ReadInstance(infile);
    auto solver = solver::BranchingSolver(i);
    auto solution = solver.solve();
    auto t1 = std::clock();
    auto t_delta_ms = ((double) (t1 - t0)) / ((double) CLOCKS_PER_SEC / 1000.0);
    std::ofstream outStream(outfile);
    outStream << "# t " << t_delta_ms << "\n# s " << solution->Roots().size() << "\n";
    solution->write(outStream);

    return 0;
}
