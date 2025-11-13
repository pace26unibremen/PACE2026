#include "Forest.hpp"

#include "ForestIO.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <utility>

using namespace std;

namespace graph
{

// ------------------------------------------------------------- //
// ---- constructors ------------------------------------------- //
// ------------------------------------------------------------- //

Forest::Forest(std::shared_ptr<std::vector<Node>> nodes,
               std::shared_ptr<std::unordered_map<int, unsigned int>> terminalIndexToLabel,
               std::shared_ptr<std::vector<int>> rootIndices) :
        nodes(std::move(nodes)),
        terminalIndexToLabel(std::move(terminalIndexToLabel)),
        rootIndices(std::move(rootIndices))
{}

Forest::Forest(const filesystem::path& path, int numberOfLeafs, int numberOfTrees)
{
    if (path.empty())
    {
        throw invalid_argument("Forest : Constructor : provided file path is empty");
    }
    ifstream file = ifstream(path);
    if (!file.is_open())
    {
        throw invalid_argument("Forest : Constructor : unable to open file");
    }
    *this = ForestIO::ReadNewick(file, numberOfLeafs, numberOfTrees);
}

// ------------------------------------------------------------- //
// ---- persistence -------------------------------------------- //
// ------------------------------------------------------------- //

void Forest::write(std::ostream& out_file) const
{
    ForestIO::WriteNewick(*this, out_file);
}

void Forest::write(const string& path) const
{
    std::ofstream outStream(path);
    if (!outStream.is_open())
    {
        throw std::invalid_argument("Forest : write : couldn't open file");
    }
    write(outStream);
    outStream.close();
}

void Forest::dot(ostream& stream) const
{
    ForestIO::WriteDot(*this, stream);
}

void Forest::dot(const string& path) const
{
    std::ofstream outStream(path);
    if (!outStream.is_open())
    {
        throw std::invalid_argument("Forest : dot : couldn't open file");
    }
    dot(outStream);
    outStream.close();
}

// ------------------------------------------------------------- //
// ---- access to member fields -------------------------------- //
// ------------------------------------------------------------- //

vector<Node>& Forest::Nodes()
{
    return *this->nodes;
}

const vector<Node>& Forest::Nodes() const
{
    return *this->nodes;
}

unordered_map<int, unsigned int>& Forest::Terminals()
{
    return *this->terminalIndexToLabel;
}

const unordered_map<int, unsigned int>& Forest::Terminals() const
{
    return *this->terminalIndexToLabel;
}

vector<int>& Forest::RootIndices()
{
    return *this->rootIndices;
}

const vector<int>& Forest::RootIndices() const
{
    return *this->rootIndices;
}

// ------------------------------------------------------------- //
// ---- debug -------------------------------------------------- //
// ------------------------------------------------------------- //

void Forest::print() const
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
    std::clog << "\n"
              << rowIndex.str() << "\n"
              << rowLine.str() << "\n"
              << rowParent.str() << "\n"
              << rowSibling.str() << "\n"
              << rowFstChild.str() << "\n"
              << rowSndChild.str() << endl;
}

bool Forest::isValid() const
{
    bool valid = true;

    for(int nodeIndex = 0; nodeIndex < (int) nodes->size(); nodeIndex++)
    {
        const Node& node = nodes->at(nodeIndex);

        if(node.parentIndex == -1 and
           node.firstChildIndex == -1 and
           node.secondChildIndex == -1)
        {
            // fully reduced node or single node tree
            continue;
        }

        // root or leaf
        if(node.firstChildIndex >= 0 and node.firstChildIndex < (int) nodes->size() and
           node.secondChildIndex >= 0 and node.secondChildIndex < (int) nodes->size())
        {
            const Node& fstChild = nodes->at(node.firstChildIndex);
            const Node& sndChild = nodes->at(node.secondChildIndex);

            // node.child.parent == node
            if(fstChild.parentIndex != nodeIndex)
            {
                std::clog << "Forest: isValid: inconstant parent-child relation:\n"
                             "   parent (" << nodeIndex << ") -> (" << node.firstChildIndex <<") child\n" <<
                             "   parent (" << fstChild.parentIndex << ") <- (" << node.firstChildIndex <<") child\n";
                valid = false;
            }
            if(sndChild.parentIndex != nodeIndex)
            {
                std::clog << "Forest: isValid: inconstant parent-child relation:\n"
                             "   parent (" << nodeIndex << ") -> (" << node.secondChildIndex <<") child\n" <<
                             "   parent (" << sndChild.parentIndex << ") <- (" << node.secondChildIndex <<") child\n";
                valid = false;
            }

            // TODO check sibling order and label structure
        }
        // node is terminal
        else if (node.firstChildIndex == -1 and node.secondChildIndex == -1)
        {
            // TODO check is terminal
        }
        else
        {
            std::clog << "Forest: isValid: invalid sibling indices:\n"
                         "   node (" << nodeIndex << "): fst Child (" << node.firstChildIndex <<") "
                        "snd Child (" << node.secondChildIndex <<") \n";
            valid = false;
        }

        // node is root node <=> no parent
        if(node.parentIndex == -1)
        {
            auto it = std::find(rootIndices->begin(), rootIndices->end(), nodeIndex);
            if(it == rootIndices->end())
            {
                std::clog << "Forest: isValid: non-root node without parent: " << nodeIndex ;
                valid = false;
            }
        }
        else if(node.parentIndex < 0 or node.parentIndex >= (int) nodes->size())
        {
            std::clog << "Forest: isValid: invalid parent index: " << nodeIndex ;
            valid = false;
        }
    }

    std::unordered_set<unsigned int> seen;
    bool uniqueTerminals = std::all_of(terminalIndexToLabel->begin(), terminalIndexToLabel->end(),
                                       [&seen](const auto& pair) { return seen.insert(pair.second).second; });
    if (not uniqueTerminals)
    {
        std::clog << "Tree: isValid: duplicate terminal labels";
        valid = false;
    }
    if(not valid)
    {
        print();
    }
    return valid;
}

// ------------------------------------------------------------- //
// ---- graph manipulation ------------------------------------- //
// ------------------------------------------------------------- //

void Forest::contractNode(int nodeIndex)
{
    if (terminalIndexToLabel->contains((int)nodeIndex))
    {
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
            auto it = std::find(rootIndices->begin(), rootIndices->end(), nodeIndex);
            if (it != rootIndices->end())
                replace(rootIndices->begin(), rootIndices->end(), nodeIndex, childIndex);
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

int Forest::removeEdge(int childIndex)
{
    Node& child = nodes->at(childIndex);
    Node& parent = nodes->at(child.parentIndex);

    nodes->at(childIndex).parentIndex = -1;
    if (parent.firstChildIndex == childIndex)
    {
        parent.firstChildIndex = -1;
    }
    else
    {
        parent.secondChildIndex = -1;
    }
//     reorder the root indices or use ordered_map for rootIndices
    rootIndices->push_back(childIndex);
    return childIndex;
}

void Forest::orderSiblings()
{
    std::function<unsigned int(int)> orderSubtree = [&](int subtree) -> unsigned int {
        Node& subtreeRoot = nodes->at(subtree);
        if (terminalIndexToLabel->contains(subtree))
        {
            return terminalIndexToLabel->at(subtree);
        }
        unsigned int firstMinLabel = orderSubtree(subtreeRoot.firstChildIndex);
        unsigned int secondMinLabel = orderSubtree(subtreeRoot.secondChildIndex);
        if (firstMinLabel > secondMinLabel)
        {
            std::swap(subtreeRoot.firstChildIndex, subtreeRoot.secondChildIndex);
        }
        return std::min(firstMinLabel, secondMinLabel);
    };
    for(const auto root : *rootIndices)
    {
        orderSubtree(root);
    }
}

// ------------------------------------------------------------- //
// ---- operators ---------------------------------------------- //
// ------------------------------------------------------------- //

bool Forest::operator==(const Forest& other) const
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
    if(rootIndices->size() != other.rootIndices->size())
    {
        return false;
    }
    for(int i = 0; i < (int)rootIndices->size(); ++i)
    {
        auto eq = compareSubtrees(rootIndices->at(i), other.rootIndices->at(i));
        if(!eq)
        {
            return false;
        }
    }
    return true;
}

}  //namespace graph
