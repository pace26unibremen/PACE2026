#include "Tree.hpp"

#include "../Logger/Logger.hpp"
#include "TreeIO.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

using namespace std;

namespace graph
{

// ------------------------------------------------------------- //
// ---- constructors ------------------------------------------- //
// ------------------------------------------------------------- //

Tree::Tree(std::shared_ptr<std::vector<Node>> nodes,
           std::shared_ptr<std::unordered_map<int, unsigned int>> terminalIndexToLabel, int rootIndex) :
        nodes(std::move(nodes)),
        terminalIndexToLabel(std::move(terminalIndexToLabel)),
        rootIndex(rootIndex)
{}

Tree::Tree(filesystem::path path)
{
    if (path.empty())
    {
        LOG_ERR_CH("Tree") << "Provided file path is empty!";
        throw invalid_argument("Tree : Constructor : provided file path is empty");
    }
    ifstream file = ifstream(path);
    if (!file.is_open())
    {
        LOG_ERR_CH("Tree") << "Unable to open file: " << path;
        throw invalid_argument("Tree : Constructor : unable to open file");
    }
    *this = TreeIO::ReadNewick(file);
}

// ------------------------------------------------------------- //
// ---- persistence -------------------------------------------- //
// ------------------------------------------------------------- //

void Tree::write(std::ostream& out_file) const
{
    TreeIO::WriteNewick(*this, out_file);
}

void Tree::write(const string& path) const
{
    std::ofstream outStream(path);
    if (!outStream.is_open())
    {
        LOG_ERR_CH("Parser") << "Tree : write : couldn't open file";
        throw std::invalid_argument("Tree : write : couldn't open file");
    }
    write(outStream);
    outStream.close();
}

void Tree::dot(ostream& stream) const
{
    TreeIO::WriteDot(*this, stream);
}

void Tree::dot(const string& path) const
{
    std::ofstream outStream(path);
    if (!outStream.is_open())
    {
        LOG_ERR_CH("Parser") << "Tree : dot : couldn't open file";
        throw std::invalid_argument("Tree : dot : couldn't open file");
    }
    dot(outStream);
    outStream.close();
}

// ------------------------------------------------------------- //
// ---- access to member fields -------------------------------- //
// ------------------------------------------------------------- //

vector<Node>& Tree::Nodes()
{
    return *this->nodes;
}

const vector<Node>& Tree::Nodes() const
{
    return *this->nodes;
}

unordered_map<int, unsigned int>& Tree::Terminals()
{
    return *this->terminalIndexToLabel;
}

const unordered_map<int, unsigned int>& Tree::Terminals() const
{
    return *this->terminalIndexToLabel;
}

int& Tree::RootIndex()
{
    return this->rootIndex;
}

const int& Tree::RootIndex() const
{
    return this->rootIndex;
}

// ------------------------------------------------------------- //
// ---- debug -------------------------------------------------- //
// ------------------------------------------------------------- //

void Tree::print() const
{
    stringstream rowIndex;
    stringstream rowLine;
    stringstream rowParent;
    stringstream rowSibling;
    stringstream rowFstChild;
    stringstream rowSndChild;

    rowIndex << "     Index |";
    rowLine << "-----------+";
    rowParent << "    Parent |";
    rowSibling << "   Sibling |";
    rowFstChild << " fst Child |";
    rowSndChild << " snd Child |";

    for (unsigned int i = 0; i < nodes->size(); i++)
    {
        const Node& n = nodes->at(i);
        if (terminalIndexToLabel->contains((int)i))
        {
            rowIndex << std::setw(3) << terminalIndexToLabel->at((int)i) << " " << std::setw(3) << i << " |";
        }
        else
        {
            rowIndex << std::setw(7) << i << " |";
        }
        rowLine << "--------+";
        rowParent << std::setw(7) << n.parentIndex << " |";
        rowSibling << std::setw(7) << n.siblingIndex << " |";
        rowFstChild << std::setw(7) << n.firstChildIndex << " |";
        rowSndChild << std::setw(7) << n.secondChildIndex << " |";
    }
    LOG_INF_CH("Tree") << "\n"
                       << rowIndex.str() << "\n"
                       << rowLine.str() << "\n"
                       << rowParent.str() << "\n"
                       << rowSibling.str() << "\n"
                       << rowFstChild.str() << "\n"
                       << rowSndChild.str() << endl;
}

bool Tree::isValid() const
{
    bool isValid = true;

    for (int i = 0; i < (int)nodes->size(); ++i)
    {
        const Node& node = nodes->at(i);
        if(node.parentIndex == -1 and node.siblingIndex == -1 and
           node.firstChildIndex == -1 and node.secondChildIndex == -1)
        {
            continue;
        }
        if (i == rootIndex)
        {
            if (node.siblingIndex != -1 or node.parentIndex != -1)
            {
                LOG_INF_CH("Tree") << "isValid: Root with sibling or parent #" << i;
            }
        }
        if (terminalIndexToLabel->contains(i))
        {
            if (node.firstChildIndex != -1 or node.secondChildIndex != -1)
            {
                LOG_INF_CH("Tree") << "isValid: Terminal with children #" << i;
            }
        }

        if (i != rootIndex)
        {
            // check for correct siblings
            if (node.siblingIndex >= (int)nodes->size() or node.siblingIndex < 0)
            {
                LOG_INF_CH("Tree") << "isValid: invalid index for sibling #" << i;
                isValid = false;
            }
            else
            {
                if (node.siblingIndex == i)
                {
                    LOG_INF_CH("Tree") << "isValid: #" << i << " is its own sibling";
                    isValid = false;
                }
                if (nodes->at(node.siblingIndex).siblingIndex != i)
                {
                    LOG_INF_CH("Tree") << "isValid: sibling of #" << i << " is not sym";
                    isValid = false;
                }
                if (nodes->at(node.siblingIndex).parentIndex != node.parentIndex)
                {
                    LOG_INF_CH("Tree") << "isValid: sibling of #" << i << " has different parents";
                    isValid = false;
                }
            }
            // check for parents
            if (node.parentIndex >= (int)nodes->size() or node.parentIndex < 0)
            {
                LOG_INF_CH("Tree") << "isValid: parent index for #" << i;
                isValid = false;
            }
            else
            {
                if (nodes->at(node.parentIndex).firstChildIndex != i and
                    nodes->at(node.parentIndex).secondChildIndex != i)
                {
                    LOG_INF_CH("Tree") << "isValid: parent of #" << i << " has it not as child";
                    isValid = false;
                }
            }
        }
    }
    std::unordered_set<unsigned int> seen;
    bool uniqueTerminals = std::all_of(terminalIndexToLabel->begin(), terminalIndexToLabel->end(),
                                       [&seen](const auto& pair) { return seen.insert(pair.second).second; });
    if (not uniqueTerminals)
    {
        LOG_INF_CH("Tree") << "isValid: duplicate terminal labels";
        isValid = false;
    }
    return isValid;
}

// ------------------------------------------------------------- //
// ---- graph manipulation ------------------------------------- //
// ------------------------------------------------------------- //

void Tree::contractNode(int nodeIndex)
{
    if (terminalIndexToLabel->contains((int)nodeIndex))
    {
        LOG_WRN_CH("Tree") << "contractNode: Cannot contract terminal node";
        return;
    }
    Node& node = nodes->at(nodeIndex);
    // case: no root, no child
    if (node.firstChildIndex == -1 and node.secondChildIndex == -1 and node.parentIndex == -1)
    {
        if (node.siblingIndex != -1)
        {
            nodes->at(node.siblingIndex).siblingIndex = -1;
            node.siblingIndex = -1;
        }
        return;
    }

    // case: one root, no child
    if (node.firstChildIndex == -1 and node.secondChildIndex == -1)
    {
        int parentIndex = node.parentIndex;
        Node& parent = nodes->at(parentIndex);

        if (parent.firstChildIndex == nodeIndex)
        {
            parent.firstChildIndex = -1;
        }
        else if (parent.secondChildIndex == nodeIndex)
        {
            parent.secondChildIndex = -1;
        }

        if (node.siblingIndex != -1)
        {
            nodes->at(node.siblingIndex).siblingIndex = -1;
            node.siblingIndex = -1;
        }
        node.parentIndex = -1;

        contractNode(parentIndex);
        return;
    }

    if (node.firstChildIndex == -1 or node.secondChildIndex == -1)
    {
        int childIndex = node.firstChildIndex == -1 ? node.secondChildIndex : node.firstChildIndex;
        Node& child = nodes->at(childIndex);

        // case: no root, one child
        if (node.parentIndex == -1)
        {
            child.parentIndex = -1;
            if (rootIndex == nodeIndex)
                rootIndex = childIndex;
            if (node.siblingIndex != -1)
            {
                nodes->at(node.siblingIndex).siblingIndex = -1;
                node.siblingIndex = -1;
            }
            node.firstChildIndex = -1;
            node.secondChildIndex = -1;
            return;
        }
        // case: one root, one child
        else
        {
            Node& parent = nodes->at(node.parentIndex);
            if (parent.firstChildIndex == nodeIndex)
            {
                parent.firstChildIndex = childIndex;
                child.siblingIndex = parent.secondChildIndex;
                nodes->at(parent.secondChildIndex).siblingIndex = childIndex;
            }
            else if (parent.secondChildIndex == nodeIndex)
            {
                parent.secondChildIndex = childIndex;
                child.siblingIndex = parent.firstChildIndex;
                nodes->at(parent.firstChildIndex).siblingIndex = childIndex;
            }
            child.parentIndex = node.parentIndex;

            node.parentIndex = -1;
            node.firstChildIndex = -1;
            node.secondChildIndex = -1;
            node.siblingIndex = -1;
        }
    }
}

std::shared_ptr<Tree> Tree::removeEdge(int childIndex)
{
    Node& child = nodes->at(childIndex);
    Node& parent = nodes->at(child.parentIndex);

    auto newTree = make_shared<Tree>(nodes, terminalIndexToLabel, childIndex);

    nodes->at(childIndex).parentIndex = -1;
    if(parent.firstChildIndex == childIndex)
    {
        parent.firstChildIndex = -1;
    }
    else
    {
        parent.secondChildIndex = -1;
    }
    return newTree;
}

void Tree::orderSiblings()
{
    std::function<unsigned int(int)> orderSubtree = [&](int subtree) -> unsigned int
    {
        Node& subtreeRoot = nodes->at(subtree);
        if (terminalIndexToLabel->contains(subtree))
        {
            return terminalIndexToLabel->at(subtree);
        }
        unsigned int firstMinLabel = orderSubtree(subtreeRoot.firstChildIndex);
        unsigned int secondMinLabel = orderSubtree(subtreeRoot.secondChildIndex);
        if(firstMinLabel > secondMinLabel)
        {
            std::swap(subtreeRoot.firstChildIndex, subtreeRoot.secondChildIndex);
//            int temp = subtreeRoot.firstChildIndex;
//            subtreeRoot.firstChildIndex = subtreeRoot.secondChildIndex;
//            subtreeRoot.secondChildIndex = temp;
        }
        return std::min(firstMinLabel, secondMinLabel);
    };
    orderSubtree(rootIndex);
}

// ------------------------------------------------------------- //
// ---- operators ---------------------------------------------- //
// ------------------------------------------------------------- //

bool Tree::operator==(const Tree& other) const
{
    std::function<bool(int, int)> compareSubtrees = [&](int thisNodeIdx, int otherNodeIdx) -> bool {
        const Node& thisNode = (*nodes)[thisNodeIdx];
        const Node& otherNode = (*other.nodes)[otherNodeIdx];

        bool thisIsTerminal = thisNode.firstChildIndex == -1;
        bool otherIsTerminal = otherNode.firstChildIndex == -1;

        if (thisIsTerminal != otherIsTerminal)
        {
            return false;
        }
        if (thisIsTerminal)
        {
            return terminalIndexToLabel->at(thisNodeIdx) == other.terminalIndexToLabel->at(otherNodeIdx);
        }
        return compareSubtrees(thisNode.firstChildIndex, otherNode.firstChildIndex) and
           compareSubtrees(thisNode.secondChildIndex, otherNode.secondChildIndex);
    };
    return compareSubtrees(rootIndex, other.rootIndex);
}

}  //namespace graph
