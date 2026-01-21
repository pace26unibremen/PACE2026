
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
               std::shared_ptr<std::unordered_map<Node*, unsigned int>> terminalToLabel,
               std::shared_ptr<std::unordered_map<unsigned int, Node*>> labelToTerminal,
               std::shared_ptr<std::vector<Node*>> roots) :
        nodes(std::move(nodes)),
        terminalToLabel(std::move(terminalToLabel)),
        labelToTerminal(std::move(labelToTerminal)),
        roots(std::move(roots))
{
    // FIXME: The function seems to have an error or is calling another function that isn't working correctly.
    // sortChildrenAndCollectTerminals();

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

std::unordered_map<Node*, unsigned int>& Forest::Terminals()
{
    return *this->terminalToLabel;
}

const std::unordered_map<Node*, unsigned int>& Forest::Terminals() const
{
    return *this->terminalToLabel;
}

std::unordered_map<unsigned int, Node*>& Forest::LabelToTerminal()
{
    return *this->labelToTerminal;
}

const std::unordered_map<unsigned int, Node*>& Forest::LabelToTerminal() const
{
    return *this->labelToTerminal;
}

std::vector<Node*>& Forest::Roots()
{
    return *this->roots;
}

const std::vector<Node*>& Forest::Roots() const
{
    return *this->roots;
}

Node* Forest::rootOf(const Node& node) const
{
    for(auto rootPtr : *roots)
    {
        const auto& root = *rootPtr;
        if(node.hasSubsetTerminals(root))
        {
            return rootPtr;
        }
    }
    assert(false);
}

Node* Forest::rootOf(Node* node) const
{
    return rootOf(*node);
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
        Node& n = nodes->at(i);
        if (terminalToLabel->contains(&n))
        {
            rowIndex << std::setw(3) << terminalToLabel->at(&n) << " " << std::setw(3) << i << " |";
        }
        else
        {
            rowIndex << std::setw(7) << i << " |";
        }
        rowLine << "--------+";
        rowParent << std::setw(7) << n.parent << " |";
        rowSibling << std::setw(7) << n.sibling << " |";
        rowFstChild << std::setw(7) << n.leftChild << " |";
        rowSndChild << std::setw(7) << n.rightChild << " |";
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
    std::unordered_map<Node*, unsigned int> totalLeafs;
    unsigned int lastSmallestTerminal = 0;
    // Check all reachable Roots
    for (Node* node : *roots)
    {
        std::unordered_map<Node*, unsigned int> rootLeafs;
        const Node& root = *node;
        unsigned int smallestTerminal = -1; // -> 4294967295
        set<Node*> pointers;
        if (root.parent != nullptr)
        {
            std::clog << "Forest: isValid: Root has a parent:\n"
                         "   parent (" << root.parent << ") -> Root (" << node <<") \n"
                         "   Let's get to the root of the issue."<< endl;
            valid = false;
        }
        // Check children (recursive)
        valid &= checkTriple(node, rootLeafs, pointers, smallestTerminal);
        totalLeafs.merge(rootLeafs);
        // Check root order
        if (smallestTerminal < lastSmallestTerminal)
        {
            std::clog << "Forest: isValid: root order disrupted:\n"
                         "   root (" << node << ") -> smallestTerminal (" << smallestTerminal <<") \n"
                         "   previous smallest leaf (" << lastSmallestTerminal <<") \n"
                         "   Order in the court!"<< endl;
            valid = false;
        }
        lastSmallestTerminal = smallestTerminal;
    }
    // Check terminal index -> label
    if (*terminalToLabel != totalLeafs)
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
    std::function<bool(Node*, Node*)> traverseUp = [&](Node* thisNodePtr, Node* otherNodePtr) -> bool {
        const Node& node = *thisNodePtr;
        const Node& otherNode = *otherNodePtr;

        if (not node.hasSameTerminals(otherNode))
        {
            return false;
        }
        if (node.parent == nullptr)
        {
            return true;
        }
        if (otherNode.parent == nullptr)
        {
            return false;
        }
        return traverseUp(node.parent, otherNode.parent);
    };

    if (this->labelToTerminal->size() > other.labelToTerminal->size())
    {
        return false;
    }

    for (auto& [key, value]: *labelToTerminal)
    {
        if (not traverseUp(value, other.labelToTerminal->at(key)))
        {
            return false;
        }
    }
    return true;
}

bool Forest::hasIdenticalSubtree(const Forest& other, Node* thisNode, Node* otherNode)
{
    std::stack<Node*> thisVisitingStack;
    thisVisitingStack.push(thisNode);
    std::stack<Node*> otherVisitingStack;
    otherVisitingStack.push(otherNode);

    while (not (thisVisitingStack.empty() or otherVisitingStack.empty()))
    {
        Node& thisCurrentNode = *thisVisitingStack.top();
        thisVisitingStack.pop();
        Node& otherCurrentNode = *otherVisitingStack.top();
        otherVisitingStack.pop();

        if (not thisCurrentNode.hasSameTerminals(otherCurrentNode))
        {
            return false;
        }

        if (not (thisCurrentNode.leftChild == nullptr or otherCurrentNode.leftChild == nullptr))
        {
            // neither is a leaf node
            thisVisitingStack.push(thisCurrentNode.rightChild);
            thisVisitingStack.push(thisCurrentNode.leftChild);
            otherVisitingStack.push(otherCurrentNode.rightChild);
            otherVisitingStack.push(otherCurrentNode.leftChild);
        }
        else
        {
            // one or both are leaves
            if (not (thisCurrentNode.leftChild == nullptr and otherCurrentNode.rightChild == nullptr))
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

bool Forest::checkTriple(Node* parentPtr, std::unordered_map<Node*, unsigned int>& subtreeLeafs, set<Node*>& pointers, unsigned int& smallestTerminal) const
{ //TODO: checkTriple
    bool tripleValid = true;
    const Node& node = *parentPtr;
    // Check index
    if (!pointers.insert(parentPtr).second) //TODO: auch nicht mehr nötig? umschreiben
    {
        std::clog << "Forest: isValid: Duplicate index:\n"
                         "   Index (" << parentPtr << ") \n"
                         "   Who is the original?"<< endl;
        tripleValid = false;
    }
    // Check Leaf
    if (node.leftChild == nullptr && node.rightChild == nullptr)
    {
        if (terminalToLabel->contains(parentPtr))
        { // Add to found leafs
            unsigned int label = terminalToLabel->at(parentPtr);
            // Check label -> index
            if (labelToTerminal->at(label) != parentPtr)
            {
                std::clog << "Forest: isValid: label to index incorrect:\n"
                         "   Node (" << parentPtr << ") -> Label ("<< label <<")\n"
                         "   Label ("<< label <<") -> Index (" << labelToTerminal->at(label) << ")\n"
                         "   At least we're about to use pointers like real c-programmers. ^^"<< endl;
                tripleValid = false;
            }
            // Save smallest terminal in tree
            if (label < smallestTerminal)
            {
                smallestTerminal = label;
            }
            subtreeLeafs.emplace(parentPtr, label);
        } else
        {
            std::clog << "Forest: isValid: Leaf has no label:\n"
                         "   Node (" << parentPtr << ") \n"
                         "   Maybe he can sign with Sony?"<< endl;
            tripleValid = false;
        }
    }
    else // Has children
    {
        //Balance
        if ((node.leftChild== nullptr && node.rightChild != nullptr)||(node.leftChild== nullptr && node.rightChild != nullptr))
        {
            std::clog << "Forest: isValid: unbalanced Tree:\n"
                         "   parent (" << parentPtr << ") -> (" << node.leftChild <<") , (" << node.rightChild <<") \n"
                         "   Parent needs to get active and produce another child."<< endl;
            tripleValid = false;
        }
        const Node& fstChild = *node.leftChild;
        const Node& sndChild = *node.rightChild;
        // Order
        if (!fstChild.hasSmallestTerminal(sndChild))
        {
            std::clog << "Forest: isValid: unordered Tree:\n"
                         "   parent (" << parentPtr << ") -> (" << node.leftChild <<") , (" << node.rightChild <<") \n"
                         "   Commander Cody, the time has come. Execute Order 66."<< endl;
            tripleValid = false;
        }
        // First child
        // Check parent
        if (fstChild.parent != parentPtr)
        {
            std::clog << "Forest: isValid: checkTriple: first child forgot his parent:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node.leftChild
                      << ") child\n"
                         "   parent ("
                      << fstChild.parent << ") <- (" << node.leftChild
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (fstChild.sibling != node.rightChild)
        {
            std::clog << "Forest: isValid: checkTriple: first child forgot his sibling:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node.leftChild << " and " << node.rightChild
                      << ") children\n"
                         "   first child ("
                      << node.leftChild << ") -> (" << fstChild.sibling
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Second child
        // Check parent
        if (sndChild.parent != parentPtr)
        {
            std::clog << "Forest: isValid: checkTriple: second child forgot his parent:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node.rightChild
                      << ") child\n"
                         "   parent ("
                      << sndChild.parent << ") <- (" << node.rightChild
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (sndChild.sibling != node.leftChild)
        {
            std::clog << "Forest: isValid: checkTriple: second child forgot his sibling:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node.leftChild << " and " << node.rightChild
                      << ") children\n"
                         "   first child ("
                      << node.rightChild << ") -> (" << sndChild.sibling
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Recursive call with children
        std::unordered_map<Node*, unsigned int> leftLeafs;
        std::unordered_map<Node*, unsigned int> rightLeafs;
        tripleValid &= checkTriple(node.leftChild, leftLeafs, pointers,smallestTerminal) && checkTriple(node.rightChild, rightLeafs, pointers,smallestTerminal);
        leftLeafs.merge(rightLeafs);
        subtreeLeafs = leftLeafs; // Collect leafs of subtree
    }
    //TODO labelToTerminalIndex check (after change index->leaf)

    vector <uint64_t> foundTerminals = {0}; //TODO: only accounts for 64(32?) terminals, need to add logic to scale the vector
    for( auto it = subtreeLeafs.begin(); it != subtreeLeafs.end(); ++it ) {
        foundTerminals[0]+= 1<<(it->second - 1);
    }
    // Compare found terminals with terminals saved in node
    if (foundTerminals != node.subtreeTerminals)
    {
        std::clog << "Forest: isValid: subtreeTerminals list is incorrect:\n"
                        "   at index (" << parentPtr << ") \n"
                        "   Maybe you should fix that? ;)"<< endl;
        tripleValid = false;
    }
    return tripleValid;
}

// ------------------------------------------------------------- //
// ---- graph manipulation ------------------------------------- //
// ------------------------------------------------------------- //


#ifdef DEBUG_IMAGE_VIEW_GRAPH //TODO: ??
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
    unsigned int length = 0;
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
    const unsigned int numberOfEntries = (terminalToLabel->size() + 63) / 64;

    std::function<unsigned int(Node*)> orderSubtree = [&, numberOfEntries](Node* subtree) -> unsigned int {
        Node& subtreeRoot = *subtree;
        subtreeRoot.subtreeTerminals.resize(numberOfEntries,0);
        if (terminalToLabel->contains(subtree))
        {
            const unsigned int label = terminalToLabel->at(subtree);
            // (label - 1) because smallest label is 1 and not 0
            subtreeRoot.subtreeTerminals[(label - 1) / 64] = (1 << (label -1) % 64);
            return label;
        }
        unsigned int firstMinLabel = orderSubtree(subtreeRoot.leftChild);
        unsigned int secondMinLabel = orderSubtree(subtreeRoot.rightChild);
        if (firstMinLabel > secondMinLabel)
        {
            std::swap(subtreeRoot.leftChild, subtreeRoot.rightChild);
        }
        for(unsigned int i = 0; i < subtreeRoot.subtreeTerminals.size(); i++)
        {
            subtreeRoot.subtreeTerminals[i] =
                subtreeRoot.leftChild->subtreeTerminals[i] |
                subtreeRoot.rightChild->subtreeTerminals[i];
        }
        return std::min(firstMinLabel, secondMinLabel);
    };
    for(const auto root : *roots)
    {
        orderSubtree(root);
    }
    std::sort(roots->begin(),roots->end(),
              [&](Node* a, Node* b)
              {
                  return a->hasSmallestTerminal(*b);
              });
}

std::vector<Node*> Forest::maximumCommonSubforestRoots(const Forest& other)
{
    // actually: iterate upward, always checking the sibling for identicality and the parent for the same terminals, deleting seen labels from the ones left to visit
    // vorinitialisierung von blattknoten; jeder durchlauf kreiert seinen sibling- und elternknoten; dabei sibling nur, wenn nicht besucht? ach nee, das ist was anderes. wenn nichtterminal? wird sowieso rekursiv/iterativ, also abbruchbedingung einfach? hilfsfunktion, die den gesamten sibling-subtree baut und den sibling-index zurückgibt (bei blatt halt nur rückgabe des index) bei leaf-siblings retrieval über label=index?
    // naur, just steal it from f1: iterate upward from each unvisited leaf, always checking the sibling subtree for identicality, and once a root for the leaf is found, do the label-index relation? should be doable via the parent forests map. make a new nodes vector after scanning the whole parent forest and exclude->reindex
    vector<Node*> newRootIndices; // indices of new root nodes in t1
    newRootIndices.reserve(labelToTerminal->size());

    // maps new forests node index to original forest indices
    unordered_map<int, int> t1Index;
    unordered_map<int, int> t2Index;

    unordered_set<unsigned int> visitedLeaves;

    for (auto& [leaf, leafLabel] : *terminalToLabel)
    {
        if (visitedLeaves.contains(leafLabel)){continue;}
        Node* t1CurrentNode = leaf;
        Node* t2CurrentNode = (*other.labelToTerminal)[leafLabel];

        while (not (t1CurrentNode->parent == nullptr or t2CurrentNode->parent == nullptr))
        { // node is root in neither tree
            if (not hasIdenticalSubtree(other, t1CurrentNode->sibling, t2CurrentNode->sibling))
            { // sibling subtree differs
                break;
            }

            t1CurrentNode = t1CurrentNode->parent;
            t2CurrentNode = t2CurrentNode->parent;
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

        newRootIndices.push_back(t1CurrentNode);
    }

    return newRootIndices;
}

// ------------------------------------------------------------- //
// ---- operators ---------------------------------------------- //
// ------------------------------------------------------------- //

bool Forest::operator==(const Forest& other) const
{
    std::function<bool(Node*, Node*)> compareSubtrees = [&](Node* thisNodePtr, Node* otherNodePtr) -> bool {
        const Node& thisNode = *thisNodePtr;
        const Node& otherNode = *otherNodePtr;

        bool thisIsTerminal = thisNode.leftChild == nullptr;
        bool otherIsTerminal = otherNode.leftChild == nullptr;

        if (thisIsTerminal != otherIsTerminal)
        {
            return false;
        }
        if (thisIsTerminal)
        {
            return terminalToLabel->at(thisNodePtr) == other.terminalToLabel->at(otherNodePtr);
        }
        return compareSubtrees(thisNode.leftChild, otherNode.leftChild) and
               compareSubtrees(thisNode.rightChild, otherNode.rightChild);
    };
    if(roots->size() != other.roots->size())
    {
        return false;
    }
    for(int i = 0; i < (int)roots->size(); ++i)
    {
        auto eq = compareSubtrees(roots->at(i), other.roots->at(i));
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
    std::shared_ptr<std::unordered_map<Node*, unsigned int>> copiedTerminalToLabel =
        std::make_shared<std::unordered_map<Node*, unsigned int>>();
    std::shared_ptr<std::unordered_map<unsigned int, Node*>> copiedLabelToTerminal =
        std::make_shared<std::unordered_map<unsigned int, Node*>>();
    std::shared_ptr<std::vector<Node*>> copiedRoots = std::make_shared<std::vector<Node*>>();
    //Start Copying the param
    //Nodes
    // Anstelle -> auch nichts: this allein| nodes, Nodes()
    copiedNodes->reserve(nodes->capacity());
    for ( Node node : *nodes)
    {
        copiedNodes->push_back(node);
    }
    //Terminal Indecies
    copiedTerminalToLabel->reserve(terminalToLabel->size());
    for ( auto termIndexToLabel : *terminalToLabel)
    {
        copiedTerminalToLabel->insert(termIndexToLabel);
    }
    copiedLabelToTerminal->reserve(labelToTerminal->size());
    //Labels for Terminal Indecies
    for (auto labelToTermIndex : *labelToTerminal)
    {
        copiedLabelToTerminal->insert(labelToTermIndex);
    }
    copiedRoots->reserve(roots->capacity());
    //Root Indices
    for (Node* root : *roots)
    {
        copiedRoots->push_back(root);
    }

    Forest newForest(copiedNodes, copiedTerminalToLabel, copiedLabelToTerminal, copiedRoots);

    return newForest;

}

}  //namespace graph
