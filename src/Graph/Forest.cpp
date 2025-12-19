
#include "Forest.hpp"

#include "ForestIO.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
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
               std::shared_ptr<std::unordered_map<unsigned int, int>> labelToTerminalIndex,
               std::shared_ptr<std::vector<int>> rootIndices) :
        nodes(std::move(nodes)),
        terminalIndexToLabel(std::move(terminalIndexToLabel)),
        labelToTerminalIndex(std::move(labelToTerminalIndex)),
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
    sortChildrenAndCollectTerminals();
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

unordered_map<unsigned int, int>& Forest::LabelToTerminalIndex()
{
    return *this->labelToTerminalIndex;
}

const unordered_map<unsigned int, int>& Forest::LabelToTerminalIndex() const
{
    return *this->labelToTerminalIndex;
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
    std::unordered_map<int, unsigned int> totalLeafs;
    unsigned int lastSmallestTerminal = 0;
    // Check all reachable Roots
    for (const int nodeIndex : *rootIndices)
    {
        std::unordered_map<int, unsigned int> rootLeafs;
        const Node& root = nodes->at(nodeIndex);
        unsigned int smallestTerminal = -1; // -> 4294967295
        set<int> indices;
        if (root.parentIndex != -1)
        {
            std::clog << "Forest: isValid: Root has a parent:\n"
                         "   parent (" << root.parentIndex << ") -> Root (" << nodeIndex <<") \n"
                         "   Let's get to the root of the issue."<< endl;
            valid = false;
        }
        // Check children (recursive)
        valid &= checkTriple(nodeIndex, rootLeafs, indices, smallestTerminal);
        totalLeafs.merge(rootLeafs);
        // Check root order
        if (smallestTerminal < lastSmallestTerminal)
        {
            std::clog << "Forest: isValid: root order disrupted:\n"
                         "   root (" << nodeIndex << ") -> smallestTerminal (" << smallestTerminal <<") \n"
                         "   previous smallest leaf (" << lastSmallestTerminal <<") \n"
                         "   Order in the court!"<< endl;
            valid = false;
        }
        lastSmallestTerminal = smallestTerminal;
    }
    // Check terminal index -> label
    if (*terminalIndexToLabel != totalLeafs)
    {
        std::clog << "Forest: isValid: unreachable leafs in terminalIndexToLabel:\n"
                     "   List Leafs? \n"
                     "   She loves me. She loves me not. She loves me. She is undefined?"<< endl;
        valid = false;
    }
    // Print forest after check (bad readability for big forests)
    // if(not valid)
    // {
    //     print();
    // }
    return valid;
}


bool Forest::checkTriple(int parentIndex, std::unordered_map<int, unsigned int>& subtreeLeafs, set<int>& indices, unsigned int& smallestTerminal) const
{
    bool tripleValid = true;
    const Node& node = nodes->at(parentIndex);
    // Check index
    if (parentIndex < 0)
    {
        std::clog << "Forest: isValid: Negative index:\n"
                         "   Index (" << parentIndex << ") \n"
                         "   Be positive! :)"<< endl;
        tripleValid = false;
    }
    if (!indices.insert(parentIndex).second)
    {
        std::clog << "Forest: isValid: Duplicate index:\n"
                         "   Index (" << parentIndex << ") \n"
                         "   Who is the original?"<< endl;
        tripleValid = false;
    }
    // Check Leaf
    if (node.firstChildIndex == -1 && node.secondChildIndex == -1)
    {
        if (terminalIndexToLabel->contains(parentIndex))
        { // Add to found leafs
            unsigned int label = terminalIndexToLabel->at(parentIndex);
            // Check label -> index
            if (labelToTerminalIndex->at(label) != parentIndex)
            {
                std::clog << "Forest: isValid: label to index incorrect:\n"
                         "   Node (" << parentIndex << ") -> Label ("<< label <<")\n"
                         "   Label ("<< label <<") -> Index (" << labelToTerminalIndex->at(label) << ")\n"
                         "   Maybe we should use pointers like real c-programmers? ^^"<< endl;
                tripleValid = false;
            }
            // Save smallest terminal in tree
            if (label < smallestTerminal)
            {
                smallestTerminal = label;
            }
            subtreeLeafs.emplace(parentIndex, label);
        } else
        {
            std::clog << "Forest: isValid: Leaf has no label:\n"
                         "   Node (" << parentIndex << ") \n"
                         "   Maybe he can sign with Sony?"<< endl;
            tripleValid = false;
        }
    }
    else // Has children
    {
        //Balance
        if ((node.firstChildIndex== -1 && node.secondChildIndex != -1)||(node.firstChildIndex== -1 && node.secondChildIndex != -1))
        {
            std::clog << "Forest: isValid: unbalanced Tree:\n"
                         "   parent (" << parentIndex << ") -> (" << node.firstChildIndex <<") , (" << node.secondChildIndex <<") \n"
                         "   Parent needs to get active and produce another child."<< endl;
            tripleValid = false;
        }
        const Node& fstChild = nodes->at(node.firstChildIndex);
        const Node& sndChild = nodes->at(node.secondChildIndex);
        // Order
        if (!fstChild.hasSmallestTerminal(sndChild))
        {
            std::clog << "Forest: isValid: unordered Tree:\n"
                         "   parent (" << parentIndex << ") -> (" << node.firstChildIndex <<") , (" << node.secondChildIndex <<") \n"
                         "   Commander Cody, the time has come. Execute Order 66."<< endl;
            tripleValid = false;
        }
        // First child
        // Check parent
        if (fstChild.parentIndex != parentIndex)
        {
            std::clog << "Forest: isValid: checkTriple: first child forgot his parent:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.firstChildIndex
                      << ") child\n"
                         "   parent ("
                      << fstChild.parentIndex << ") <- (" << node.firstChildIndex
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (fstChild.siblingIndex != node.secondChildIndex)
        {
            std::clog << "Forest: isValid: checkTriple: first child forgot his sibling:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.firstChildIndex << " and " << node.secondChildIndex
                      << ") children\n"
                         "   first child ("
                      << node.firstChildIndex << ") -> (" << fstChild.siblingIndex
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Second child
        // Check parent
        if (sndChild.parentIndex != parentIndex)
        {
            std::clog << "Forest: isValid: checkTriple: second child forgot his parent:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.secondChildIndex
                      << ") child\n"
                         "   parent ("
                      << sndChild.parentIndex << ") <- (" << node.secondChildIndex
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (sndChild.siblingIndex != node.firstChildIndex)
        {
            std::clog << "Forest: isValid: checkTriple: second child forgot his sibling:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.firstChildIndex << " and " << node.secondChildIndex
                      << ") children\n"
                         "   first child ("
                      << node.secondChildIndex << ") -> (" << sndChild.siblingIndex
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Recursive call with children
        std::unordered_map<int, unsigned int> leftLeafs;
        std::unordered_map<int, unsigned int> rightLeafs;
        tripleValid &= checkTriple(node.firstChildIndex, leftLeafs, indices,smallestTerminal) && checkTriple(node.secondChildIndex, rightLeafs, indices,smallestTerminal);
        leftLeafs.merge(rightLeafs);
        subtreeLeafs = leftLeafs; // Collect leafs of subtree
    }
    //TODO labelToTerminalIndex check (after change index->leaf)

    vector <uint64_t> foundTerminals = {0};
    for( auto it = subtreeLeafs.begin(); it != subtreeLeafs.end(); ++it ) {
        foundTerminals[0]+= 1<<(it->second - 1);
    }
    // Compare found terminals with terminals saved in node
    if (foundTerminals != node.subtreeTerminals)
    {
        std::clog << "Forest: isValid: subtreeTerminals list is incorrect:\n"
                        "   at index (" << parentIndex << ") \n"
                        "   Maybe you should fix that? ;)"<< endl;
        tripleValid = false;
    }
    return tripleValid;
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
        // first new root is always at same position than the parent
        *it = parent.firstChildIndex;

        // position of second new root is somewhere after the old parent
        auto it2 = std::lower_bound(
            it, rootIndices->end(), parent.secondChildIndex,
            [&](const int& a, const int& b)
            {
                const Node& an = nodes->at(a);
                const Node& bn = nodes->at(b);
                return an.hasSmallestTerminal(bn);
            }
        );
        rootIndices->insert(it2, parent.secondChildIndex);

        sibling.parentIndex = -1;
        sibling.siblingIndex = -1;
    }
    // case 2: parent is inner node
    else
    {
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

        const unsigned int subtreeTerminalsSize = child.subtreeTerminals.size();
        int traverseUpIndex = child.parentIndex;
        int rootIndex;
        while(traverseUpIndex >= 0)
        {
            Node& traversedNode = nodes->at(traverseUpIndex);
            rootIndex = traverseUpIndex;
            for (unsigned int i = 0; i < subtreeTerminalsSize; i++)
            {
                traversedNode.subtreeTerminals[i] ^= child.subtreeTerminals[i];
            }
            traverseUpIndex = traversedNode.parentIndex;
            // sort children
            const Node& l = nodes->at(traversedNode.firstChildIndex);
            const Node& r = nodes->at(traversedNode.secondChildIndex);
            if(r.hasSmallestTerminal(l))
            {
                swap(traversedNode.firstChildIndex, traversedNode.secondChildIndex);
            }
        }

        auto itRoot = std::find(rootIndices->begin(), rootIndices->end(),rootIndex);
        auto rootNode = nodes->at(*itRoot);

        if(rootNode.hasSmallestTerminal(child))
        {
            auto itNewRoot =
                std::lower_bound(itRoot, rootIndices->end(), childIndex, [&](const int& a, const int& b)
                     {
                        const Node& an = nodes->at(a);
                        const Node& bn = nodes->at(b);
                        return an.hasSmallestTerminal(bn);
                     });
            rootIndices->insert(itNewRoot, childIndex);
        }
        else
        {
            *itRoot = childIndex;
            auto itRootNewPosition =
                std::lower_bound(itRoot, rootIndices->end(), rootIndex, [&](const int& a, const int& b)
                    {
                        const Node& an = nodes->at(a);
                        const Node& bn = nodes->at(b);
                        return an.hasSmallestTerminal(bn);
                    });
            rootIndices->insert(itRootNewPosition, rootIndex);
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
    std::sort(rootIndices->begin(),rootIndices->end(),
              [&](int a, int b)
              {
                  return nodes->at(a).hasSmallestTerminal(nodes->at(b));
              });
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
// ------------------------------------------------------------- //
// ---- copy func ---------------------------------------------- //
// ------------------------------------------------------------- //

Forest Forest::copy()
{

    //Init empty Forest
    std::shared_ptr<std::vector<Node>> copiedNodes = std::make_shared<std::vector<Node>>();
    std::shared_ptr<std::unordered_map<int, unsigned int>> copiedTerminalIndexToLabel =
        std::make_shared<std::unordered_map<int, unsigned int>>();
    std::shared_ptr<std::unordered_map<unsigned int, int>> copiedLabelToTerminalIndex =
        std::make_shared<std::unordered_map<unsigned int, int>>();
    std::shared_ptr<std::vector<int>> copiedRootIndices = std::make_shared<std::vector<int>>();
    //Start Copying the param
    //Nodes
    // Anstelle -> auch nichts: this allein| nodes, Nodes()
    copiedNodes->reserve(nodes->capacity());
    for ( Node node : *nodes)
    {
        copiedNodes->push_back(node);
    }
    //Terminal Indecies
    copiedTerminalIndexToLabel->reserve(terminalIndexToLabel->size());
    for ( auto termIndexToLabel : *terminalIndexToLabel)
    {
        copiedTerminalIndexToLabel->insert(termIndexToLabel);
    }
    copiedLabelToTerminalIndex->reserve(labelToTerminalIndex->size());
    //Labels for Terminal Indecies
    for (auto labelToTermIndex : *labelToTerminalIndex)
    {
        copiedLabelToTerminalIndex->insert(labelToTermIndex);
    }
    copiedRootIndices->reserve(rootIndices->capacity());
    //Root Indices
    for (int rootIndex : *rootIndices)
    {
        copiedRootIndices->push_back(rootIndex);
    }

    Forest newForest(copiedNodes, copiedTerminalIndexToLabel, copiedLabelToTerminalIndex, copiedRootIndices);

    return newForest;

}

}  //namespace graph
