#include "TrivialSolver.hpp"

#include <vector>
#include <numeric>

solver::TrivialSolver::TrivialSolver(graph::Instance& instance)
{
    this->instance = std::move(instance);
}

graph::Forest solver::TrivialSolver::solve()
{
    unsigned int numberOfTerminals = instance[0]->Terminals().size();

    auto nodes = std::make_shared<std::vector<graph::Node>>(numberOfTerminals);
    auto roots = std::make_shared<std::vector<int>>(numberOfTerminals);
    auto indexToTerminalLabel = std::make_shared<std::unordered_map<int, unsigned int>>(numberOfTerminals);

    int index = 0;
    for(const auto & [_, label] : instance[0]->Terminals())
    {
        nodes->at(index) = {-1,-1,-1,-1};
        roots->at(index) = index;
        indexToTerminalLabel->emplace(index++,label);
    }
    return {nodes, indexToTerminalLabel, roots};
}