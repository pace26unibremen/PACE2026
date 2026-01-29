#include "DebugPlugin.hpp"

#include "Rule/AbstractBranchingRule.hpp"

#include <iostream>
#include <utility>
#include <algorithm>

solver::DebugPlugin::DebugPlugin(std::string _dirPath)
{
    dirPath = std::move(_dirPath);
    std::filesystem::create_directories(dirPath);
    overviewFile = std::ofstream(dirPath + "/overview.dot");
    if (!overviewFile.is_open())
    {
        throw std::invalid_argument("DebugPlugin : constructor : couldn't open file");
    }
}

void solver::DebugPlugin::writeStateNode()
{
    overviewFile << "state_" << stateIDs.top()
              << " [style=filled, "
                 "fillcolor=lightblue, "
                 "width=1,"
                 "label = \"" << stateIDs.top() << "\n";
    for (auto& f : *instance)
    {
        overviewFile << f->RootIndices().size() << " ";
    }
    overviewFile << "\"];\n";
}

void solver::DebugPlugin::writeRuleNode(const std::shared_ptr<AbstractRule>& rule)
{
    if (const auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
    {
        overviewFile << "subgraph rule_" << stateIDs.top() << " {\n"
        << "  cluster=true;\n"
        << "  width=3.5;\n"
        << "  label=\"" << branchingRule->name() << "\";\n"
        << "  style=\"filled,rounded\";\n"
        << "  fillcolor=orange;\n";
        for (int branchId = 1; branchId <= branchingRule->MaxBranch(); branchId++)
        {
            overviewFile << "  rule_" << stateIDs.top() << "_" << branchId
                         << " [label=" << branchId << ", style=\"filled,dotted,rounded\", "
                                                      "fillcolor=darkorange, width=0.9];\n";
        }
        overviewFile << "}\n;";
    }
    else
    {
        overviewFile << "rule_" << stateIDs.top()
        << R"( [style="filled,rounded", fillcolor=orange, width=3.5, label=")" << rule->name() << "\"]\n";
    }
}

void solver::DebugPlugin::dotInstance(const std::filesystem::path& path)
{
    std::ofstream os(path);
    if (!os.is_open())
    {
        throw std::invalid_argument("DebugPlugin : dotInstance : couldn't open file");
    }

    os << "digraph Instance {\n"
       << "splines = false\n\n";

    for (unsigned int i = 0; i < shadowInstance->size(); ++i)
    {
        std::shared_ptr<graph::Forest> forest = shadowInstance->at(i);
        auto found = std::find(instance->begin(), instance->end(), forest) != instance->end();

        os << "subgraph forest_" << i << " {\n"
           << (found ? "    style = dotted;\n" : "    style = \"dotted,filled\";\n")
           << "    cluster = true;\n"
           << "    node [\n"
           << "        shape = circle,\n"
           << "        fontsize = 15,\n"
           << "        label = \"\",\n"
           << "        height = 0.1,\n"
           << "        fillcolor = \"#00000022\",\n"
           << "        style = filled,\n"
           << "        fixedsize = true,\n"
           << "        labelloc = t];\n"
           << "    edge [arrowhead = none];\n\n";

        os << "    inv_" << i << "[style = invis];\n\n";

        for (auto t : forest->Terminals())
        {
            os << "    n_" << i << "_" << t.first << " [label = \"" << t.second << "\\n\\n\\n \"];\n";
            os << "    n_" << i << "_" << t.first << " -> inv_" << i << "[style = invis];\n";
        }
        for (size_t n = 0; n < forest->Nodes().size(); ++n)
        {
            const graph::Node& node = forest->Nodes()[n];
            if(node.leftChildIndex != -1)
                os << "    n_" << i << "_" << n << " -> n_" << i << "_" << node.leftChildIndex << ";\n";
            if(node.rightChildIndex != -1)
                os << "    n_" << i << "_" << n << " -> n_" << i << "_" << node.rightChildIndex << ";\n";
        }

        os << "}\n\n";
    }

    os << "}" << std::endl;
    os.close();
}

void solver::DebugPlugin::init(const std::shared_ptr<graph::Instance>& _instance)
{
    shadowInstance = std::make_shared<graph::Instance>();
    instance = _instance;

    for (auto& f : *_instance) shadowInstance->push_back(f);

    overviewFile << "digraph overview {\n"
                 << "splines=ortho;\n"
                 << "node [shape=box, margin=\"0.1,0.1\"];\n"
                 << "initial [label=\"\", shape=doublecircle, style=filled, fillcolor=green];\n";
    stateIDs.push(0);
    writeStateNode();
    overviewFile << "initial -> state_0 [URL=\"start.dot.svg\"]\n";
    dotInstance(dirPath + "/start.dot");
}

void solver::DebugPlugin::onApply(const std::shared_ptr<solver::AbstractRule>& rule)
{
    writeRuleNode(rule);
    maxStateId++;
    if (auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
    {
        overviewFile << "rule_" << stateIDs.top() << "_" << branchingRule->Branch()
                     << " -> state_" << maxStateId<<
                        " [URL=\"rule_" << stateIDs.top() << "_" << branchingRule->Branch()<<"_a.dot.svg\", target=F];\n";
        overviewFile << "state_" << stateIDs.top()
                     << " -> rule_" << stateIDs.top() << "_" << branchingRule->Branch()
                     << " [style=invis]";
        dotInstance(dirPath + "/rule_" + std::to_string(stateIDs.top()) +
            "_" + std::to_string(branchingRule->Branch()) + "_a.dot");
    }
    else
    {
        overviewFile << "rule_" << stateIDs.top()
                     << " -> state_" << maxStateId <<
                        " [URL=\"rule_" << stateIDs.top() << "_a.dot.svg\", target=F];\n";
        overviewFile << "state_" << stateIDs.top()
                     << " -> rule_" << stateIDs.top()
                     << " [style=invis]";
        dotInstance(dirPath + "/rule_" + std::to_string(stateIDs.top()) + "_a.dot");
    }
    stateIDs.push(maxStateId);
    writeStateNode();
}

void solver::DebugPlugin::onUnapply(const std::shared_ptr<solver::AbstractRule>& rule)
{
    stateIDs.pop();
    if (auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
    {
        overviewFile << "state_" << stateIDs.top()
                     << " -> rule_" << stateIDs.top() << "_" << branchingRule->Branch()
                     << " [dir=back, URL=\"rule_" << stateIDs.top() << "_" << branchingRule->Branch()<<"_u.dot.svg\", target=F];\n";
        dotInstance(dirPath + "/rule_" + std::to_string(stateIDs.top()) +
            "_" + std::to_string(branchingRule->Branch()) + "_u.dot");
    }
    else
    {
        overviewFile
                    << "state_" << stateIDs.top()
                    << "-> rule_" << stateIDs.top()
                    << " [dir=back, URL=\"rule_" << stateIDs.top() << "_u.dot.svg\", target=F];\n";
        dotInstance(dirPath + "/rule_" + std::to_string(stateIDs.top()) + "_u.dot");
    }
}

void solver::DebugPlugin::onEnd()
{
    overviewFile << "}\n";
    overviewFile.close();
}

void solver::DebugPlugin::onTempUnapply(const std::shared_ptr<solver::AbstractRule>& rule, bool lastRule)
{
    maxStateId++;
    if (lastRule)
    {
        overviewFile << "state_" << maxStateId << "[shape=doublecircle, label=\"\", style=filled, fillcolor=red];\n";
    }
    else
    {
        overviewFile << "state_" << maxStateId << "[shape=diamond, label=\"\", width=0.3, height=0.3];\n";
    }

    overviewFile << "state_" << stateIDs.top() << " -> "
                 << "state_" << maxStateId << "[URL=\"temp_" << stateIDs.top() << "_u.dot.svg\"];\n";
    dotInstance(dirPath + "/temp_" + std::to_string(stateIDs.top()) + "_u.dot");
    stateIDs.push(maxStateId);
}

void solver::DebugPlugin::onTempApply(const std::shared_ptr<solver::AbstractRule>& rule)
{
    int old = stateIDs.top();
    stateIDs.pop();
    overviewFile << "state_" << stateIDs.top() << " -> "
                 << "state_" << old << "[dir=back, URL=\"temp_" << stateIDs.top() << "_a.dot.svg\"];\n";
    dotInstance(dirPath + "/temp_" + std::to_string(stateIDs.top()) + "_a.dot");
}