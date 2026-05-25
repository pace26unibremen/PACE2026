#include "Solver/BranchingSolver.hpp"
#include <atomic>
#include <csignal>
#include <fstream>
#include <iostream>

#ifdef _POSIX_VERSION
static std::atomic<bool> g_timeout{false};
static void sigtermHandler(int) { g_timeout.store(true, std::memory_order_relaxed); }
#endif

void runOnStream(std::istream& inStream, std::ostream& outStream) {
    // Start Timer
    auto startTime = std::clock();
    auto instance = graph::ReadInstance(inStream);
    auto solver = solver::BranchingSolver(instance);

#ifdef _POSIX_VERSION
    std::signal(SIGTERM, sigtermHandler);
    solver.setTimeoutFlag(&g_timeout);
#endif

    auto solved = solver.solve();
    if (!solved)
    {
        // The timeout flag fired before any solution candidate was found.
        // This can happen when SIGTERM arrives very early in a run or when
        // the instance takes a long time to reach its first solution candidate.
        std::clog << "Solver stopped without producing a solution\n";
        return;
    }

    solver.unapplyReductions();
    auto endTime = std::clock();
    auto time_delta_ms = ((double) (endTime - startTime)) / ((double) CLOCKS_PER_SEC / 1000.0);

    outStream << "# t " << time_delta_ms << "\n# s " << solver.Instance()->at(0)->Roots().size() << "\n";
    solver.Instance()->at(0)->write(outStream);
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
