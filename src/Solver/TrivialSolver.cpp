#include "TrivialSolver.hpp"

solver::TrivialSolver::TrivialSolver(const std::shared_ptr<graph::Instance>& instance)
    : AbstractSolver(instance)
{}

bool solver::TrivialSolver::solve()
{
    unsigned int numberOfTerminals = instance->at(0)->TerminalToLabel().size();

    auto nodes = std::make_shared<std::vector<graph::Node>>(numberOfTerminals);
    auto roots = std::make_shared<std::vector<graph::Node*>>(numberOfTerminals);
    auto terminalToLabel = std::make_shared<graph::TerminalToLabelView>();
    auto labelToTerminal = std::make_shared<graph::LabelToTerminalMap>();
    labelToTerminal->reserve(numberOfTerminals);

    int index = 0;
    for (graph::Node& originalTerminal : *instance->at(0)->Nodes())
    {
        if (originalTerminal.label == 0) { continue; }  // iterate the original's terminals
        const unsigned int label = originalTerminal.label;
        graph::Node& node = nodes->at(index);
        node= {nullptr,nullptr,nullptr,nullptr};
        roots->at(index) = &node;
        terminalToLabel->emplace(&node,label);
        labelToTerminal->emplace(label,&node);
        index++;
    }
    *instance = {std::make_shared<graph::Forest>(nodes, terminalToLabel, labelToTerminal, roots)};

    return true;
}