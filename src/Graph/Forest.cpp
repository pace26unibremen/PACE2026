
#include "Forest.hpp"

#include "ForestIO.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stack>
#include <unordered_set>
#include <utility>
#include <stack>
#ifdef DEBUG_IMAGE_VIEW_GRAPH
#include <graphviz/gvc.h>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>
#include <fcntl.h>
#endif

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
{
    sortChildrenAndCollectTerminals();

    #ifdef DEBUG_IMAGE_VIEW_GRAPH
    renderImage();
    #endif
}

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

int Forest::rootIndexOf(const Node& node) const
{
    for(auto index : *rootIndices)
    {
        const auto& root = Nodes()[index];
        if(node.hasSubsetTerminals(root))
        {
            return index;
        }
    }
    assert(false);
}

int Forest::rootIndexOf(int nodeIndex) const
{
    return rootIndexOf(Nodes()[nodeIndex]);
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
        rowFstChild << std::setw(7) << n.leftChildIndex << " |";
        rowSndChild << std::setw(7) << n.rightChildIndex << " |";
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

bool Forest::isTrueSubtreeOf(const Forest& other) const
{
    std::function<bool(int, int)> traverseUp = [&](int thisNodeIdx, int otherNodeIdx) -> bool {
        const Node& node = (*nodes)[thisNodeIdx];
        const Node& otherNode = (*other.nodes)[otherNodeIdx];

        if (not node.hasSameTerminals(otherNode))
        {
            return false;
        }
        if (node.parentIndex == -1)
        {
            return true;
        }
        if (otherNode.parentIndex == -1)
        {
            return false;
        }
        return traverseUp(node.parentIndex, otherNode.parentIndex);
    };

    if (this->labelToTerminalIndex->size() > other.labelToTerminalIndex->size())
    {
        return false;
    }

    for (auto& [key, value]: *labelToTerminalIndex)
    {
        if (not traverseUp(value, other.labelToTerminalIndex->at(key)))
        {
            return false;
        }
    }
    return true;
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

        if (not (thisCurrentNode.leftChildIndex == -1 or otherCurrentNode.leftChildIndex == -1))
        {
            // neither is a leaf node
            thisVisitingStack.push(thisCurrentNode.rightChildIndex);
            thisVisitingStack.push(thisCurrentNode.leftChildIndex);
            otherVisitingStack.push(otherCurrentNode.rightChildIndex);
            otherVisitingStack.push(otherCurrentNode.leftChildIndex);
        }
        else
        {
            // one or both are leaves
            if (not (thisCurrentNode.leftChildIndex == -1 and otherCurrentNode.rightChildIndex == -1))
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
    if (node.leftChildIndex == -1 && node.rightChildIndex == -1)
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
        if ((node.leftChildIndex== -1 && node.rightChildIndex != -1)||(node.leftChildIndex== -1 && node.rightChildIndex != -1))
        {
            std::clog << "Forest: isValid: unbalanced Tree:\n"
                         "   parent (" << parentIndex << ") -> (" << node.leftChildIndex <<") , (" << node.rightChildIndex <<") \n"
                         "   Parent needs to get active and produce another child."<< endl;
            tripleValid = false;
        }
        const Node& fstChild = nodes->at(node.leftChildIndex);
        const Node& sndChild = nodes->at(node.rightChildIndex);
        // Order
        if (!fstChild.hasSmallestTerminal(sndChild))
        {
            std::clog << "Forest: isValid: unordered Tree:\n"
                         "   parent (" << parentIndex << ") -> (" << node.leftChildIndex <<") , (" << node.rightChildIndex <<") \n"
                         "   Commander Cody, the time has come. Execute Order 66."<< endl;
            tripleValid = false;
        }
        // First child
        // Check parent
        if (fstChild.parentIndex != parentIndex)
        {
            std::clog << "Forest: isValid: checkTriple: first child forgot his parent:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.leftChildIndex
                      << ") child\n"
                         "   parent ("
                      << fstChild.parentIndex << ") <- (" << node.leftChildIndex
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (fstChild.siblingIndex != node.rightChildIndex)
        {
            std::clog << "Forest: isValid: checkTriple: first child forgot his sibling:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.leftChildIndex << " and " << node.rightChildIndex
                      << ") children\n"
                         "   first child ("
                      << node.leftChildIndex << ") -> (" << fstChild.siblingIndex
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
                      << parentIndex << ") -> (" << node.rightChildIndex
                      << ") child\n"
                         "   parent ("
                      << sndChild.parentIndex << ") <- (" << node.rightChildIndex
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (sndChild.siblingIndex != node.leftChildIndex)
        {
            std::clog << "Forest: isValid: checkTriple: second child forgot his sibling:\n"
                         "   parent ("
                      << parentIndex << ") -> (" << node.leftChildIndex << " and " << node.rightChildIndex
                      << ") children\n"
                         "   first child ("
                      << node.rightChildIndex << ") -> (" << sndChild.siblingIndex
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Recursive call with children
        std::unordered_map<int, unsigned int> leftLeafs;
        std::unordered_map<int, unsigned int> rightLeafs;
        tripleValid &= checkTriple(node.leftChildIndex, leftLeafs, indices,smallestTerminal) && checkTriple(node.rightChildIndex, rightLeafs, indices,smallestTerminal);
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


#ifdef DEBUG_IMAGE_VIEW_GRAPH
void Forest::renderImage()
{
    std::stringstream dotRep;
    this->dot(dotRep);

    GVC_t* gvc = gvContext();
    Agraph_t* g = agmemread(dotRep.str().c_str());

    int old_stderr = dup(STDERR_FILENO);
    int devnull = open("/dev/null", O_WRONLY);
    dup2(devnull, STDERR_FILENO);
    close(devnull);
    gvLayout(gvc, g, "dot");
    dup2(old_stderr, STDERR_FILENO);
    close(old_stderr);

    char* data = nullptr;
    size_t length = 0;
    gvRenderData(gvc, g, "png", &data, &length);

    std::vector<uchar> pngData(data, data + length);
    cv::Mat img = cv::imdecode(pngData, cv::IMREAD_UNCHANGED);

    gvFreeRenderData(data);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);

    this->image = img;
}
#endif

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
        unsigned int firstMinLabel = orderSubtree(subtreeRoot.leftChildIndex);
        unsigned int secondMinLabel = orderSubtree(subtreeRoot.rightChildIndex);
        if (firstMinLabel > secondMinLabel)
        {
            std::swap(subtreeRoot.leftChildIndex, subtreeRoot.rightChildIndex);
        }
        for(unsigned int i = 0; i < subtreeRoot.subtreeTerminals.size(); i++)
        {
            subtreeRoot.subtreeTerminals[i] =
                nodes->at(subtreeRoot.leftChildIndex).subtreeTerminals[i] |
                nodes->at(subtreeRoot.rightChildIndex).subtreeTerminals[i];
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

std::vector<int> Forest::maximumCommonSubforestRoots(const Forest& other)
{
    // actually: iterate upward, always checking the sibling for identicality and the parent for the same terminals, deleting seen labels from the ones left to visit
    // vorinitialisierung von blattknoten; jeder durchlauf kreiert seinen sibling- und elternknoten; dabei sibling nur, wenn nicht besucht? ach nee, das ist was anderes. wenn nichtterminal? wird sowieso rekursiv/iterativ, also abbruchbedingung einfach? hilfsfunktion, die den gesamten sibling-subtree baut und den sibling-index zurückgibt (bei blatt halt nur rückgabe des index) bei leaf-siblings retrieval über label=index?
    // naur, just steal it from f1: iterate upward from each unvisited leaf, always checking the sibling subtree for identicality, and once a root for the leaf is found, do the label-index relation? should be doable via the parent forests map. make a new nodes vector after scanning the whole parent forest and exclude->reindex
    auto newNodeIndices = make_shared<unordered_set<int>>();
    vector<int> newRootIndices; // indices of new root nodes in t1

    newNodeIndices->reserve(2 * labelToTerminalIndex->size() - 1);
    newRootIndices.reserve(labelToTerminalIndex->size());

    // maps new forests node index to original forest indices
    unordered_map<int, int> t1Index;
    unordered_map<int, int> t2Index;

    unordered_set<unsigned int> visitedLeaves;

    for (auto& [leafIndex, leafLabel] : *terminalIndexToLabel)
    {
        if (visitedLeaves.contains(leafLabel)){continue;}
        Node* t1CurrentNode = &nodes->at(leafIndex);
        int t1CurrentIndex = leafIndex;
        Node* t2CurrentNode = &other.nodes->at((*other.labelToTerminalIndex)[leafLabel]);

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
        unsigned int label = 1;
        for (uint64_t& i : t1CurrentNode->subtreeTerminals)
        {
            uint64_t temp = i;
            while (temp >= 1)
            {
                if ((temp & 1) == 1){visitedLeaves.insert(label);}
                label++;
                temp = temp >> 1;
            }
            label = label + (65 - (label % 64));
        }

        newRootIndices.push_back(t1CurrentIndex);
    }


    return newRootIndices;
}

// ------------------------------------------------------------- //
// ---- operators ---------------------------------------------- //
// ------------------------------------------------------------- //

bool Forest::operator==(const Forest& other) const
{
    std::function<bool(int, int)> compareSubtrees = [&](int thisNodeIdx, int otherNodeIdx) -> bool {
        const Node& thisNode = (*nodes)[thisNodeIdx];
        const Node& otherNode = (*other.nodes)[otherNodeIdx];

        bool thisIsTerminal = thisNode.leftChildIndex == -1;
        bool otherIsTerminal = otherNode.leftChildIndex == -1;

        if (thisIsTerminal != otherIsTerminal)
        {
            return false;
        }
        if (thisIsTerminal)
        {
            return terminalIndexToLabel->at(thisNodeIdx) == other.terminalIndexToLabel->at(otherNodeIdx);
        }
        return compareSubtrees(thisNode.leftChildIndex, otherNode.leftChildIndex) and
               compareSubtrees(thisNode.rightChildIndex, otherNode.rightChildIndex);
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
