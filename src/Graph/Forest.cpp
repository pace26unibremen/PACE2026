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

Forest::Forest(const filesystem::path& path, int numberOfTerminals, int numberOfTrees)
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
    *this = ForestIO::ReadNewick(file, numberOfTerminals, numberOfTrees);
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

void Forest::removeEdge(int childIndex)
{
    Node& child = nodes->at(childIndex);
    Node& sibling = nodes->at(child.siblingIndex);
    Node& parent = nodes->at(child.parentIndex);

    auto it = find(rootIndices->begin(),rootIndices->end(), child.parentIndex);

    // case 1: parent is root
    if(it != rootIndices->end())
    {
        rootIndices->at(*it) = parent.firstChildIndex;
        rootIndices->push_back(parent.secondChildIndex);
        sibling.parentIndex = -1;
        sibling.siblingIndex = -1;
    }
    // case 2: parent is inner node
    else
    {
        rootIndices->push_back(childIndex);
        Node& grandParent = nodes->at(parent.parentIndex);
        if (grandParent.firstChildIndex == child.parentIndex)
        {
            grandParent.firstChildIndex = child.siblingIndex;
        }
        else
        {
            grandParent.secondChildIndex = child.siblingIndex;
        }
        sibling.parentIndex = parent.parentIndex;
        sibling.siblingIndex = parent.siblingIndex;
        nodes->at(parent.siblingIndex).siblingIndex = child.siblingIndex;

        int iterateUpwards = child.parentIndex;
        const unsigned int subtreeTerminalsSize = child.subtreeTerminals.size();
        while(iterateUpwards >= 0)
        {
            Node& itNode = nodes->at(iterateUpwards);
            for(unsigned int i = 0; i < subtreeTerminalsSize; i++)
            {
                itNode.subtreeTerminals[i] ^= child.subtreeTerminals[i];
                iterateUpwards = itNode.parentIndex;
            }
        }

    }
    // clean up refs
    child.siblingIndex = -1;
    child.parentIndex = -1;
    parent.firstChildIndex = -1;
    parent.secondChildIndex = -1;
    parent.parentIndex = -1;
    parent.siblingIndex = -1;
}

void Forest::sortChildrenAndCollectTerminals()
{
    // the number of elements in the `subtreeTerminals` vector of each node
    const unsigned int numberOfEntries = (terminalIndexToLabel->size() + 63) / 64;

    std::function<unsigned int(int)> orderSubtree = [&, numberOfEntries](int subtree) -> unsigned int {
        Node& subtreeRoot = nodes->at(subtree);
        subtreeRoot.subtreeTerminals.resize(numberOfEntries,0);
        if (terminalIndexToLabel->contains(subtree))
        {
            const unsigned int label = terminalIndexToLabel->at(subtree);
            // (label - 1) because smallest label is 1 and not 0
            subtreeRoot.subtreeTerminals[(label - 1) / 64] = (1 << (label -1) % 64);
            return label;
        }
        unsigned int firstMinLabel = orderSubtree(subtreeRoot.firstChildIndex);
        unsigned int secondMinLabel = orderSubtree(subtreeRoot.secondChildIndex);
        if (firstMinLabel > secondMinLabel)
        {
            std::swap(subtreeRoot.firstChildIndex, subtreeRoot.secondChildIndex);
        }
        for(unsigned int i = 0; i < subtreeRoot.subtreeTerminals.size(); i++)
        {
            subtreeRoot.subtreeTerminals[i] =
                nodes->at(subtreeRoot.firstChildIndex).subtreeTerminals[i] |
                nodes->at(subtreeRoot.secondChildIndex).subtreeTerminals[i];
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
