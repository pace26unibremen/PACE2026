#include "ForestIO.hpp"

#include <fstream>
#include <stack>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace graph;

Forest ForestIO::ReadNewick(std::istream& stream, int numberOfTerminals, int numberOfTrees)
{
    auto terminalToLabel = make_shared<unordered_map<Node*, unsigned int>>();
    auto labelToTerminal = make_shared<unordered_map<unsigned int, Node*>>();
    auto nodes = make_shared<vector<Node>>();
    auto roots = make_shared<vector<Node*>>();

    // IMPORTANT for pointer stability:
    // emplace_back() can reallocate the vector; that would invalidate Node* pointers.
    // If you can estimate an upper bound (like 2*n-1 for a binary tree),
    if(numberOfTerminals > 0)
    {
        nodes->reserve(2* numberOfTerminals - 1);
        terminalToLabel->reserve(numberOfTerminals);
        labelToTerminal->reserve(numberOfTerminals);
    }

    // Stack of "currently open" internal nodes:
    // When we see '(' we create an internal node and push it.
    // When we see ')' we close that subtree and pop it.
    stack<Node*> parentStack;

    // Tracks the previously created child under the current parent, so we can connect siblings correctly.
    Node* sibling = nullptr;

    for(int treeCounter = 0; treeCounter < numberOfTrees; treeCounter++)
    {
        string line;

        // First node we create for this tree will be its root
        Node* currentRoot = nullptr;

        while (getline(stream, line))
        {
            // Skip empty lines and comments
            if (line.empty())
                continue;
            if (line[0] == '#')
                continue;

            std::string label;

            // Parse the Newick format line
            for (const char& c : line)
            {
                switch (c)
                {
                    // End of tree
                    case ';':
                    {
                        // If there is a pending label right before ';', emit that leaf.
                        if (not label.empty())
                        {
                            // Create new internal node
                            nodes->emplace_back();
                            Node* terminal = &nodes->back();

                            // Link to parent (if any)
                            if (not parentStack.empty())
                            {
                                Node* parent = parentStack.top();
                                terminal->parent = parent;

                                // Attach as left child if it's the first child,
                                // otherwise as right child and set sibling links.
                                if (sibling == nullptr)
                                {
                                    parent->leftChild = terminal;
                                }
                                else
                                {
                                    parent->rightChild = terminal;
                                    sibling->sibling = terminal;
                                    terminal->sibling = sibling;
                                }
                            }

                            // If the root is not yet assigned, this is the first node -> root
                            if (currentRoot == nullptr)
                                currentRoot = terminal;

                            sibling = nullptr;

                            unsigned int lab = static_cast<unsigned int>(std::stoi(label));
                            terminalToLabel->emplace(terminal, lab);
                            labelToTerminal->emplace(lab, terminal);
                            label.clear();
                        }

                        goto endTree;
                    }

                    // Separator between two children
                    case ',':
                    {
                        // If we have collected digits, that means we just finished a terminal label.
                        if (not label.empty())
                        {
                            // Create new internal node
                            nodes->emplace_back();
                            Node* terminal = &nodes->back();

                            // Link to parent (if any)
                            if (not parentStack.empty())
                            {
                                Node* parent = parentStack.top();
                                terminal->parent = parent;

                                // Attach as left child if it's the first child,
                                // otherwise as right child and set sibling links.
                                if (sibling == nullptr)
                                {
                                    parent->leftChild = terminal;
                                }
                                else
                                {
                                    parent->rightChild = terminal;
                                    sibling->sibling = terminal;
                                    terminal->sibling = sibling;
                                }
                            }

                            // If the root is not yet assigned, this is the first node -> root
                            if (currentRoot == nullptr)
                                currentRoot = terminal;

                            // This terminal becomes the "previous child" before the comma,
                            // so the next child can link sibling pointers to it.
                            sibling = terminal;

                            unsigned int lab = static_cast<unsigned int>(std::stoi(label));
                            terminalToLabel->emplace(terminal, lab);
                            labelToTerminal->emplace(lab, terminal);
                            label.clear();
                        }
                        else
                        {
                            // comma right after ')' etc. means "next child" but no terminal label to emit
                            sibling = nullptr;
                        }
                        break;
                    }

                    // End subtree
                    case ')':
                    {
                        // If there is a pending terminal label before ')', emit that terminal first.
                        if (not label.empty())
                        {
                            // Create new internal node
                            nodes->emplace_back();
                            Node* terminal = &nodes->back();

                            // Link to parent (if any)
                            if (not parentStack.empty())
                            {
                                Node* parent = parentStack.top();
                                terminal->parent = parent;

                                // Attach as left child if it's the first child,
                                // otherwise as right child and set sibling links.
                                if (sibling == nullptr)
                                {
                                    parent->leftChild = terminal;
                                }
                                else
                                {
                                    parent->rightChild = terminal;
                                    sibling->sibling = terminal;
                                    terminal->sibling = sibling;
                                }
                            }

                            // If the root is not yet assigned, this is the first node -> root
                            if (currentRoot == nullptr)
                                currentRoot = terminal;

                            unsigned int lab = static_cast<unsigned int>(std::stoi(label));
                            terminalToLabel->emplace(terminal, lab);
                            labelToTerminal->emplace(lab, terminal);
                            label.clear();
                        }

                        // Close the current parent subtree
                        if (not parentStack.empty())
                        {
                            // After closing, the "sibling base" becomes the node we closed,
                            // matching the original index-based logic.
                            sibling = parentStack.top();
                            parentStack.pop();
                        }
                        else
                        {
                            sibling = nullptr;
                        }
                        break;
                    }

                    // Start subtree: create a new internal node.
                    case '(':
                    {
                        // Create new internal node
                        nodes->emplace_back();
                        Node* terminal = &nodes->back();

                        // Link to parent (if any)
                        if (not parentStack.empty())
                        {
                            Node* parent = parentStack.top();
                            terminal->parent = parent;

                            // Attach as left child if it's the first child,
                            // otherwise as right child and set sibling links.
                            if (sibling == nullptr)
                            {
                                parent->leftChild = terminal;
                            }
                            else
                            {
                                parent->rightChild = terminal;
                                sibling->sibling = terminal;
                                terminal->sibling = sibling;
                            }
                        }

                        // If this is the first node we created for this tree,
                        // treat it as the tree root candidate.
                        if (currentRoot == nullptr)
                            currentRoot = terminal;

                        // We are entering a new child list under this new internal node,
                        // so reset sibling tracking and push this node as current parent.
                        sibling = nullptr;
                        parentStack.push(terminal);
                        break;
                    }

                    // Collect digits for terminal labels
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
        endTree:
        {
            // nothing to do here, start reading next tree
            // Prepare for the next tree

            // Finalize this tree: store its root pointer.
            if (currentRoot != nullptr)
                roots->push_back(currentRoot);

            // Reset stacks and pointers for the next tree
            parentStack = stack<Node*>();
            sibling = nullptr;
        };
    }
    return {nodes, terminalToLabel, labelToTerminal, roots};
}

void ForestIO::WriteNewick(const Forest& tree, std::ostream& stream)
{
    const auto& terminals = tree.Terminals();

    for(Node* current : tree.Roots())
    {
        int down = true; // traversing tree structure downwards
        while (true)
        {
            if (down)
            {
                if (current->leftChild != nullptr)
                {
                    stream << "(";
                    current = current->leftChild;
                }
                else
                {
                    stream << terminals.at(current);
                    down = false;
                }
            }
            else
            {
                if (current->parent == nullptr) break;
                if (current == current->parent->leftChild)
                {
                    stream << ",";
                    current = current->sibling;
                    down = true;
                }
                else
                {
                    stream << ")";
                    current = current->parent;
                }
            }
        }
        stream << ";\n";
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

    // FIXME: Besides the naming this doesn't seem to handle all edges correctly.
    //  At least the resulting dot file looks wrong, while the debugging output seems correct.
    for (auto node : tree.Nodes())
    {
        //FIXME: After the refactor from indices to Node Pointer this will probably use the memory addresses
        // instead of the prior indices. We need some kind of representation for all nodes here,
        // Labels shouldn't work, as not all nodes are terminals. In theory the memory address could be used though.
        // (btw. why do we got almost the exact same function twice. Instance.ccp features a similar one)
        // const Node& node = tree.Nodes()[i];
        if(node.leftChild != nullptr)
            stream << "n" << &node << " -> n" << node.leftChild << ";\n";
        if(node.rightChild != nullptr)
            stream << "n" << &node << " -> n" << node.rightChild << ";\n";
    }

    stream << "}" << endl;
}
