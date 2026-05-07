#include "Solver/BranchingSolver.hpp"
#include "Solver/ReductionSolver.hpp"
#include <fstream>
#include <iostream>

void runOnStream(std::istream& inStream, std::ostream& outStream) {
    // Start Timer
    auto startTime = std::clock();
    auto instance = graph::ReadInstance(inStream);

    auto config = std::make_shared<solver::SolverConfiguration>();
    config->subtreeReduction = true;
    config->clusterReduction = true;
    config->boundedDephtSearch = false;

    auto rs = solver::ReductionSolver(instance, config);
    auto bs = solver::BranchingSolver(instance, config);

    rs.solve();
    bs.solve();
    bs.unapplyReductions();
    rs.unapplyReductions();

    auto endTime = std::clock();
    auto timeDelta = (double) (endTime - startTime) / (double) CLOCKS_PER_SEC;

    outStream << "# t " << timeDelta << "\n# s " << instance->at(0)->Roots().size() << "\n";
    instance->at(0)->write(outStream);
}

int main(int argc, char* argv[]) {

    // For an unknown reason optil.io gives one empty string as an argument, but the input is still in stdin
    // Therefore we use this edge case but still start on the stream like this
    if (argc == 1 || (argc == 2 && std::string(argv[1]).empty()))
    {
        // With no arguments, read from stdin and write to stdout for stride benchmark
        runOnStream(std::cin, std::cout);
        return 0;
    }

    // Else run with files
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
        throw std::invalid_argument("Wrong number of arguments");
    }

    std::ifstream inStream(infile);
    std::ofstream outStream(outfile);
    runOnStream(inStream, outStream);

    return 0;
}
