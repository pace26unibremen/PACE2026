#include "TreeIO.hpp"

#include <fstream>
#include <stack>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace graph;

Tree TreeIO::ReadNewick(std::istream& stream)
{
    auto terminalIndexToLabel = make_shared<unordered_map<int, unsigned int>>();
    auto nodes = make_shared<vector<Node>>();

    stack<int> parentIndexStack;
    int siblingIndex = -1;
    int currentIndex = 0;

    string line;
    while (getline(stream, line))
    {
        if(line.empty()) continue;
        if(line[0] == '#') continue;
        std::string label;
        for (const char& c : line)
        {
            switch (c)
            {
                case ';': return {nodes, terminalIndexToLabel, 0};
                case ',':
                    if(not label.empty())
                    {
                        nodes->emplace_back();
                        if (not parentIndexStack.empty())
                        {
                            nodes->at(currentIndex).parentIndex = parentIndexStack.top();
                            if (siblingIndex == -1)
                            {
                                nodes->at(parentIndexStack.top()).firstChildIndex = currentIndex;
                            }
                            else
                            {
                                nodes->at(parentIndexStack.top()).secondChildIndex = currentIndex;
                                nodes->at(siblingIndex).siblingIndex = currentIndex;
                                nodes->at(currentIndex).siblingIndex = siblingIndex;
                            }
                        }
                        siblingIndex = currentIndex;
                        terminalIndexToLabel->emplace(currentIndex, stoi(label));
                        label = "";
                        currentIndex++;
                    }
                    break;
                case ')':
                    if(not label.empty())
                    {
                        nodes->emplace_back();
                        if (not parentIndexStack.empty())
                        {
                            nodes->at(currentIndex).parentIndex = parentIndexStack.top();
                            if (siblingIndex == -1)
                            {
                                nodes->at(parentIndexStack.top()).firstChildIndex = currentIndex;
                            }
                            else
                            {
                                nodes->at(parentIndexStack.top()).secondChildIndex = currentIndex;
                                nodes->at(siblingIndex).siblingIndex = currentIndex;
                                nodes->at(currentIndex).siblingIndex = siblingIndex;
                            }
                        }
                        terminalIndexToLabel->emplace(currentIndex, stoi(label));
                        label = "";
                        currentIndex++;
                    }
                    siblingIndex = parentIndexStack.top();
                    parentIndexStack.pop();
                    break;
                case '(':
                    nodes->emplace_back();
                    if (not parentIndexStack.empty())
                    {
                        nodes->at(currentIndex).parentIndex = parentIndexStack.top();
                        if (siblingIndex == -1)
                        {
                            nodes->at(parentIndexStack.top()).firstChildIndex = currentIndex;
                        }
                        else
                        {
                            nodes->at(parentIndexStack.top()).secondChildIndex = currentIndex;
                            nodes->at(siblingIndex).siblingIndex = currentIndex;
                            nodes->at(currentIndex).siblingIndex = siblingIndex;
                        }
                    }
                    siblingIndex = -1;
                    parentIndexStack.push(currentIndex);
                    currentIndex++;
                    break;
                default:
                    if(not isdigit(c))
                    {
                        throw invalid_argument("TreeParser : parse : unexpected symbol");
                    }
                    label += c;
                    break;
            }
        }
    }
    throw invalid_argument("TreeParser : parse : unexpected end of file");
}

void TreeIO::WriteNewick(const Tree& tree, std::ostream& out)
{
    const auto& nodes = tree.Nodes();
    const auto& terminals = tree.Terminals();
    int current = tree.RootIndex();
    int down = true; // traversing tree structure downwards

    while (true)
    {
        const Node& node = nodes[current];
        if (down)
        {
            if (node.firstChildIndex != -1)
            {
                out << "(";
                current = node.firstChildIndex;
            }
            else
            {
                out << terminals.at(current);
                down = false;
            }
        }
        else
        {
            if (current == nodes[node.parentIndex].firstChildIndex)
            {
                out << ",";
                current = node.siblingIndex;
                down = true;
            }
            else
            {
                if (node.parentIndex == -1) break;
                out << ")";
                current = node.parentIndex;
            }
        }
    }
    out << ";";
}

void TreeIO::WriteDot(const Tree& tree, ostream& stream)
{
    stream << "digraph Tree {\n"
           << "node [shape = point]\n"
           << "edge [arrowhead = none]\n"
           // << "nodesep = 0.0\n"
           // << "ranksep = 0.0\n"
           << "splines = false\n\n";
    stream << "\n{\nnode [shape = plaintext]\n";
    for (auto i : tree.Terminals())
    {
        stream << "n" << i.first << " [label = \"" << i.second << "\"];\n";
    }
    stream << "}\n";
    for (size_t i = 0; i < tree.Nodes().size(); ++i)
    {
        const Node& node = tree.Nodes()[i];
        if(node.firstChildIndex != -1)
            stream << "n" << i << " -> n" << node.firstChildIndex << ";\n";
        if(node.secondChildIndex != -1)
            stream << "n" << i << " -> n" << node.secondChildIndex << ";\n";
    }
    stream << "}" << endl;
}
