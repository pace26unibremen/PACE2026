#include "ForestIO.hpp"

#include <fstream>
#include <stack>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace graph;

Forest ForestIO::ReadNewick(std::istream& stream, int numberOfTerminals, int numberOfTrees)
{
    auto terminalIndexToLabel = make_shared<unordered_map<int, unsigned int>>();
    auto nodes = make_shared<vector<Node>>();
    auto roots = make_shared<vector<int>>();

    if(numberOfTerminals > 0)
    {
        nodes->reserve(2* numberOfTerminals - 1);
        terminalIndexToLabel->reserve(2* numberOfTerminals -1);
    }

    stack<int> parentIndexStack;
    int siblingIndex = -1;
    int currentIndex = 0;

    for(int treeCounter = 0; treeCounter < numberOfTrees; treeCounter++)
    {
        roots->push_back(currentIndex);
        string line;
        while (getline(stream, line))
        {
            if (line.empty())
                continue;
            if (line[0] == '#')
                continue;
            std::string label;
            for (const char& c : line)
            {
                switch (c)
                {
                    case ';':
                        if (not label.empty())
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
                            siblingIndex = -1;
                            terminalIndexToLabel->emplace(currentIndex, stoi(label));
                            label = "";
                            currentIndex++;
                        }
                        goto endWhile;
                    case ',':
                        if (not label.empty())
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
                        if (not label.empty())
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
                        if (not isdigit(c))
                        {
                            throw invalid_argument("TreeIO : ReadNewick : unexpected symbol");
                        }
                        label += c;
                        break;
                }
            }
        }
        endWhile:
        {
            // nothing to do here, start reading next tree
        };
    }
    return {nodes, terminalIndexToLabel, roots};
}

void ForestIO::WriteNewick(const Forest& tree, std::ostream& out)
{
    const auto& nodes = tree.Nodes();
    const auto& terminals = tree.Terminals();

    for(int current : tree.RootIndices())
    {
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
                if (node.parentIndex == -1) break;
                if (current == nodes[node.parentIndex].firstChildIndex)
                {
                    out << ",";
                    current = node.siblingIndex;
                    down = true;
                }
                else
                {
                    out << ")";
                    current = node.parentIndex;
                }
            }
        }
        out << ";\n";
    }
}

void ForestIO::WriteDot(const Forest& tree, ostream& stream)
{
    stream << "digraph Tree {\n"
           // << "nodesep = 0.0\n"
           // << "ranksep = 0.0\n"
           << "splines = false\n\n"
           << "node [\n"
           << "    shape = circle,\n"
           << "    fontsize = 15,\n"
           << "    label = \"\",\n"
           << "    height = 0.1,\n"
           << "    fillcolor = \"#00000022\",\n"
           << "    style = filled,\n"
           << "    fixedsize = true,\n"
           << "    labelloc = t];\n"
           << "edge [arrowhead = none];\n\n";

    stream << "inv[style = invis];\n\n";

    for (auto i : tree.Terminals())
    {
        stream << "n" << i.first << " [label = \"" << i.second << "\\n\\n\\n \"];\n";
        stream << "n" << i.first << " -> inv [style = invis];\n";
    }
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
