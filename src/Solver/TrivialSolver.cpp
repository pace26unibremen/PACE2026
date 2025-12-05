#include "TrivialSolver.hpp"

#include <vector>
#include <numeric>

solver::TrivialSolver::TrivialSolver(const std::shared_ptr<graph::Instance>& instance)
{
    this->instance = instance;
}

std::shared_ptr<graph::Forest> solver::TrivialSolver::solve()
{
    unsigned int numberOfTerminals = instance->at(0)->Terminals().size();

    auto nodes = std::make_shared<std::vector<graph::Node>>(numberOfTerminals);
    auto roots = std::make_shared<std::vector<int>>(numberOfTerminals);
    auto indexToLabel = std::make_shared<std::unordered_map<int, unsigned int>>(numberOfTerminals);
    auto labelToIndex = std::make_shared<std::unordered_map<unsigned int, int>>(numberOfTerminals);

    int index = 0;
    for(const auto & [_, label] : instance->at(0)->Terminals())
    {
        nodes->at(index) = {-1,-1,-1,-1};
        roots->at(index) = index;
        indexToLabel->emplace(index++,label);
        labelToIndex->emplace(index++,label);
    }
    return std::make_shared<graph::Forest>(nodes, indexToLabel, labelToIndex, roots);
}