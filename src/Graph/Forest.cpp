
#include "Forest.hpp"

#include "ForestIO.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <queue>
#include <ranges>
#include <sstream>
#include <stack>
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

bool Forest::hasIdenticalSubtree(const Forest& other, int thisNodeIdx, int otherNodeIdx)
{
    std::stack<int> thisVisitingStack;
    thisVisitingStack.push(thisNodeIdx);
    std::stack<int> otherVisitingStack;
    otherVisitingStack.push(otherNodeIdx);

    while (not (thisVisitingStack.empty() or otherVisitingStack.empty()))
    {
        Node& thisCurrentNode = nodes->at(thisVisitingStack.top());
        thisVisitingStack.pop();
        Node& otherCurrentNode = other.nodes->at(otherVisitingStack.top());
        otherVisitingStack.pop();

        if (not thisCurrentNode.hasSameTerminals(otherCurrentNode))
        {
            return false;
        }

        if (not (thisCurrentNode.firstChildIndex == -1 or otherCurrentNode.secondChildIndex == -1)) //TODO: assume both child indices -1 if one is empty?
        {
            // neither is a leaf node
            thisVisitingStack.push(thisCurrentNode.secondChildIndex);
            thisVisitingStack.push(thisCurrentNode.firstChildIndex);
            otherVisitingStack.push(otherCurrentNode.secondChildIndex);
            otherVisitingStack.push(otherCurrentNode.firstChildIndex);
        }
        else
        {
            // one or both are leaves
            if (not (thisCurrentNode.firstChildIndex == -1 and otherCurrentNode.secondChildIndex == -1))
            {
                // just one is a leaf
                return false;
            }
        }


    }
    if (thisVisitingStack.empty() and otherVisitingStack.empty())
    {
        return true;
    }
    return false;
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

std::vector<int>& Forest::maximumCommonSubforestRoots(const Forest& other)
{
    // actually: iterate upward, always checking the sibling for identicality and the parent for the same terminals, deleting seen labels from the ones left to visit
    // vorinitialisierung von blattknoten; jeder durchlauf kreiert seinen sibling- und elternknoten; dabei sibling nur, wenn nicht besucht? ach nee, das ist was anderes. wenn nichtterminal? wird sowieso rekursiv/iterativ, also abbruchbedingung einfach? hilfsfunktion, die den gesamten sibling-subtree baut und den sibling-index zurückgibt (bei blatt halt nur rückgabe des index) bei leaf-siblings retrieval über label=index?
    // naur, just steal it from f1: iterate upward from each unvisited leaf, always checking the sibling subtree for identicality, and once a root for the leaf is found, do the label-index relation? should be doable via the parent forests map. make a new nodes vector after scanning the whole parent forest and exclude->reindex
    auto newNodeIndices = make_shared<unordered_set<int>>();
    auto newRootIndices = make_shared<vector<int>>(); // indices of new root nodes in t1

    newNodeIndices->reserve(2 * labelToTerminalIndex->size() - 1);
    newRootIndices->reserve(labelToTerminalIndex->size());

    // maps new forests node index to original forest indices
    unordered_map<int, int> t1Index;
    unordered_map<int, int> t2Index;

    list<int> toVisit;
    unordered_set<unsigned int> visitedLeaves;

    int newNodeIndex = 0;


    for (auto& [leafIndex, leafLabel] : *terminalIndexToLabel)
    {
        //TODO: populate and check for visited leaves (in the sibling subforest; check via Node::subtreeTerminals)
        Node* t1CurrentNode = &nodes->at(leafIndex);
        int t1CurrentIndex = leafIndex;
        Node* t2CurrentNode = &other.nodes->at(other.labelToTerminalIndex->at(leafLabel));

        while (not (t1CurrentNode->parentIndex == -1 or t2CurrentNode->parentIndex == -1))
        { // node is root in neither tree
            if (not hasIdenticalSubtree(other, t1CurrentNode->siblingIndex, t2CurrentNode->siblingIndex))
            { // sibling subtree differs
                break;
            }

            t1CurrentIndex = t1CurrentNode->parentIndex;
            t1CurrentNode = &nodes->at(t1CurrentNode->parentIndex);
            t2CurrentNode = &other.nodes->at(t2CurrentNode->parentIndex);
        }
        // currentNode is now the root of the common subtree
        newRootIndices->push_back(t1CurrentIndex);
    }


    return *newRootIndices;
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
