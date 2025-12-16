#include "Solver/BranchingSolver.hpp"

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


    auto i = graph::ReadInstance(infile);
    auto solver = solver::BranchingSolver(i);
    auto solution = solver.solve();
    solution->write(outfile);

    return 0;
}
