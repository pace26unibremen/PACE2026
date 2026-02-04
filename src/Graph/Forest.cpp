
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

void Forest::write(std::ostream& stream) const
{
    ForestIO::WriteNewick(*this, stream);
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
        if(node.hasSubsetTerminals(rootPtr))
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
    for (Node* rootPtr : *roots)
    {
        std::unordered_map<Node*, unsigned int> rootLeafs;
        unsigned int smallestTerminal = -1; // -> max value of unsigned int
        set<Node*> pointers;
        if (rootPtr->parent != nullptr)
        {
            std::clog << "Forest: isValid: Root has a parent:\n"
                         "   parent (" << rootPtr->parent << ") -> Root (" << rootPtr <<") \n"
                         "   Let's get to the root of the issue."<< endl;
            valid = false;
        }
        // Check children (recursive)
        valid &= checkTriple(rootPtr, rootLeafs, pointers, smallestTerminal);
        totalLeafs.merge(rootLeafs);
        // Check root order
        if (smallestTerminal < lastSmallestTerminal)
        {
            std::clog << "Forest: isValid: root order disrupted:\n"
                         "   root (" << rootPtr << ") -> smallestTerminal (" << smallestTerminal <<") \n"
                         "   previous smallest leaf (" << lastSmallestTerminal <<") \n"
                         "   Order in the court!"<< endl;
            valid = false;
        }
        lastSmallestTerminal = smallestTerminal;
    }
    // Check terminal -> label
    if (*terminalToLabel != totalLeafs)
    {
        std::clog << "Forest: isValid: unreachable leafs in terminalToLabel:\n"
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
    std::function<bool(Node*, Node*)> traverseUp = [&](const Node* thisNodePtr, const Node* otherNodePtr) -> bool {

        if (not thisNodePtr->hasSameTerminals(otherNodePtr))
        {
            return false;
        }
        if (thisNodePtr->parent == nullptr)
        {
            return true;
        }
        if (otherNodePtr->parent == nullptr)
        {
            return false;
        }
        return traverseUp(thisNodePtr->parent, otherNodePtr->parent);
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

bool Forest::hasIdenticalSubtree(Node* thisNodePtr, Node* otherNodePtr)
{
    std::stack<Node*> thisVisitingStack;
    thisVisitingStack.push(thisNodePtr);
    std::stack<Node*> otherVisitingStack;
    otherVisitingStack.push(otherNodePtr);

    while (not (thisVisitingStack.empty() or otherVisitingStack.empty()))
    {
        const auto thisCurrentNodePtr = thisVisitingStack.top();
        thisVisitingStack.pop();
        const auto otherCurrentNotePtr = otherVisitingStack.top();
        otherVisitingStack.pop();

        if (not thisCurrentNodePtr->hasSameTerminals(otherCurrentNotePtr))
        {
            return false;
        }

        if (not (thisCurrentNodePtr->leftChild == nullptr or otherCurrentNotePtr->leftChild == nullptr))
        {
            // neither is a leaf node
            thisVisitingStack.push(thisCurrentNodePtr->rightChild);
            thisVisitingStack.push(thisCurrentNodePtr->leftChild);
            otherVisitingStack.push(otherCurrentNotePtr->rightChild);
            otherVisitingStack.push(otherCurrentNotePtr->leftChild);
        }
        else
        {
            // one or both are leaves
            if (not (thisCurrentNodePtr->leftChild == nullptr and otherCurrentNotePtr->rightChild == nullptr))
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
{
    bool tripleValid = true;
    const auto node = parentPtr;
    // Check pointers
    if (!pointers.insert(parentPtr).second)
    {
        std::clog << "Forest: checkTriple: Node appears second time in tree:\n"
                         "   Pointer (" << parentPtr << ") \n"
                         "   Who is the original?"<< endl;
        tripleValid = false;
    }
    // Check Leaf
    if (node->leftChild == nullptr && node->rightChild == nullptr)
    {
        if (terminalToLabel->contains(parentPtr))
        { // Add to found leafs
            unsigned int label = terminalToLabel->at(parentPtr);
            // Check label -> ptr
            if (labelToTerminal->at(label) != parentPtr)
            {
                std::clog << "Forest: checkTriple: label to terminal incorrect:\n"
                         "   Node (" << parentPtr << ") -> Label ("<< label <<")\n"
                         "   Label ("<< label <<") -> Node (" << labelToTerminal->at(label) << ")\n"
                         "   Is this unrequited love?"<< endl;
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
            std::clog << "Forest: checkTriple: Leaf has no label:\n"
                         "   Node (" << parentPtr << ") \n"
                         "   Maybe he can sign with Sony?"<< endl;
            tripleValid = false;
        }
    }
    else // Has children
    {
        //Balance
        if ((node->leftChild == nullptr && node->rightChild != nullptr)||(node->leftChild != nullptr && node->rightChild == nullptr))
        {
            std::clog << "Forest: checkTriple: unbalanced Tree:\n"
                         "   parent (" << parentPtr << ") -> (" << node->leftChild <<") , (" << node->rightChild <<") \n"
                         "   Parent needs to get active and produce another child."<< endl;
            tripleValid = false;
        }
        const auto fstChildNodePtr = node->leftChild;
        const auto sndChildNodePtr = node->rightChild;
        // Order
        if (!fstChildNodePtr->hasSmallestTerminal(sndChildNodePtr))
        {
            std::clog << "Forest: checkTriple: unordered Tree:\n"
                         "   parent (" << parentPtr << ") -> (" << node->leftChild <<") , (" << node->rightChild <<") \n"
                         "   Commander Cody, the time has come. Execute Order 66."<< endl;
            tripleValid = false;
        }
        // First child
        // Check parent
        if (fstChildNodePtr->parent != parentPtr)
        {
            std::clog << "Forest: checkTriple: first child forgot his parent:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node->leftChild
                      << ") child\n"
                         "   parent ("
                      << fstChildNodePtr->parent << ") <- (" << node->leftChild
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (fstChildNodePtr->sibling != node->rightChild)
        {
            std::clog << "Forest: checkTriple: first child forgot his sibling:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node->leftChild << " and " << node->rightChild
                      << ") children\n"
                         "   first child ("
                      << node->leftChild << ") -> (" << fstChildNodePtr->sibling
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Second child
        // Check parent
        if (sndChildNodePtr->parent != parentPtr)
        {
            std::clog << "Forest: checkTriple: second child forgot his parent:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node->rightChild
                      << ") child\n"
                         "   parent ("
                      << sndChildNodePtr->parent << ") <- (" << node->rightChild
                      << ") child\n"
                         "   Why bother raising them if they forget about you?"
                      << endl;
            tripleValid = false;
        }
        // Check sibling
        if (sndChildNodePtr->sibling != node->leftChild)
        {
            std::clog << "Forest: checkTriple: second child forgot his sibling:\n"
                         "   parent ("
                      << parentPtr << ") -> (" << node->leftChild << " and " << node->rightChild
                      << ") children\n"
                         "   first child ("
                      << node->rightChild << ") -> (" << sndChildNodePtr->sibling
                      << ") sibling"
                         "   Maybe they had a fight?"
                      << endl;
            tripleValid = false;
        }
        // Recursive call with children
        std::unordered_map<Node*, unsigned int> leftLeafs;
        std::unordered_map<Node*, unsigned int> rightLeafs;
        tripleValid &= checkTriple(node->leftChild, leftLeafs, pointers,smallestTerminal) && checkTriple(node->rightChild, rightLeafs, pointers,smallestTerminal);
        leftLeafs.merge(rightLeafs);
        subtreeLeafs = leftLeafs; // Collect leafs of subtree
    }
    //TODO labelToTerminalIndex check (after change index->leaf)

    vector <uint64_t> foundTerminals = {0}; //TODO: only accounts for 64(32?) terminals, need to add logic to scale the vector
    for( auto it = subtreeLeafs.begin(); it != subtreeLeafs.end(); ++it ) {
        foundTerminals[0]+= 1<<(it->second - 1);
    }
    // Compare found terminals with terminals saved in node
    if (foundTerminals != node->subtreeTerminals)
    {
        std::clog << "Forest: checkTriple: subtreeTerminals list is incorrect:\n"
                        "   at pointer (" << parentPtr << ") \n"
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

void Forest::sortChildrenAndCollectTerminals() //TODO check if std:swap still works
{
    // the number of elements in the `subtreeTerminals` vector of each node
    const unsigned int numberOfEntries = (terminalToLabel->size() + 63) / 64;

    std::function<unsigned int(Node*)> orderSubtree = [&, numberOfEntries](Node* subtreePtr) -> unsigned int {
        subtreePtr->subtreeTerminals.resize(numberOfEntries,0);
        if (terminalToLabel->contains(subtreePtr))
        {
            const unsigned int label = terminalToLabel->at(subtreePtr);
            // (label - 1) because smallest label is 1 and not 0
            subtreePtr->subtreeTerminals[(label - 1) / 64] = (1 << (label -1) % 64);
            return label;
        }
        unsigned int firstMinLabel = orderSubtree(subtreePtr->leftChild);
        unsigned int secondMinLabel = orderSubtree(subtreePtr->rightChild);
        if (firstMinLabel > secondMinLabel)
        {
            std::swap(subtreePtr->leftChild, subtreePtr->rightChild);
        }
        for(unsigned int i = 0; i < subtreePtr->subtreeTerminals.size(); i++)
        {
            subtreePtr->subtreeTerminals[i] =
                subtreePtr->leftChild->subtreeTerminals[i] |
                subtreePtr->rightChild->subtreeTerminals[i];
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
                  return a->hasSmallestTerminal(b);
              });
}

std::vector<Node*> Forest::maximumCommonSubforestRoots(const Forest& other)
{
    // actually: iterate upward, always checking the sibling for identicality and the parent for the same terminals, deleting seen labels from the ones left to visit
    // vorinitialisierung von blattknoten; jeder durchlauf kreiert seinen sibling- und elternknoten; dabei sibling nur, wenn nicht besucht? ach nee, das ist was anderes. wenn nichtterminal? wird sowieso rekursiv/iterativ, also abbruchbedingung einfach? hilfsfunktion, die den gesamten sibling-subtree baut und den sibling-index zurückgibt (bei blatt halt nur rückgabe des index) bei leaf-siblings retrieval über label=index?
    // naur, just steal it from f1: iterate upward from each unvisited leaf, always checking the sibling subtree for identicality, and once a root for the leaf is found, do the label-index relation? should be doable via the parent forests map. make a new nodes vector after scanning the whole parent forest and exclude->reindex
    vector<Node*> newRootPtrs; // pointers of new root nodes in t1
    newRootPtrs.reserve(labelToTerminal->size());

    // maps new forests node index to original forest indices
    unordered_map<int, int> t1Index;
    unordered_map<int, int> t2Index;

    unordered_set<unsigned int> visitedLeaves;

    for (auto& [leafPtr, leafLabel] : *terminalToLabel)
    {
        if (visitedLeaves.contains(leafLabel)){continue;}
        Node* t1CurrentNodePtr = leafPtr;
        Node* t2CurrentNodePtr = (*other.labelToTerminal)[leafLabel];

        while (not (t1CurrentNodePtr->parent == nullptr or t2CurrentNodePtr->parent == nullptr))
        { // node is root in neither tree
            if (not hasIdenticalSubtree(t1CurrentNodePtr->sibling, t2CurrentNodePtr->sibling))
            { // sibling subtree differs
                break;
            }

            t1CurrentNodePtr = t1CurrentNodePtr->parent;
            t2CurrentNodePtr = t2CurrentNodePtr->parent;
        }
        // currentNode is now the root of the common subtree
        unsigned int label = 1;
        for (uint64_t& i : t1CurrentNodePtr->subtreeTerminals)
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

        newRootPtrs.push_back(t1CurrentNodePtr);
    }

    return newRootPtrs;
}

// ------------------------------------------------------------- //
// ---- operators ---------------------------------------------- //
// ------------------------------------------------------------- //

bool Forest::operator==(const Forest& other) const
{
    std::function<bool(Node*, Node*)> compareSubtrees = [&](Node* thisNodePtr, Node* otherNodePtr) -> bool {
        const Node& thisNode = *thisNodePtr;
        const Node& otherNode = *otherNodePtr;

        auto random = rand();

        bool thisIsTerminal = thisNode.leftChild == nullptr && thisNode.rightChild == nullptr;
        bool otherIsTerminal = otherNode.leftChild == nullptr && otherNode.rightChild == nullptr;

        if (thisIsTerminal != otherIsTerminal)
        {
            return false;
        }
        if (thisIsTerminal && otherIsTerminal)
        {

            if (not terminalToLabel->contains(thisNodePtr))
            {
                std::cerr << random << " : TerminalToLabel of the Forest does not contain " << thisNodePtr << "\n";
                return false;
            }

            if (not other.terminalToLabel->contains(otherNodePtr))
            {
                std::cerr << random << " : TerminalToLabel of the other Forest does not contain " << otherNodePtr << "\n";
                return false;
            }

            if (terminalToLabel->contains(thisNodePtr) and other.terminalToLabel->contains(otherNodePtr))
            {
                return  terminalToLabel->at(thisNodePtr) == other.terminalToLabel->at(otherNodePtr);
            }

            return false;

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
    //Terminals
    copiedTerminalToLabel->reserve(terminalToLabel->size());
    for ( auto termToLabel : *terminalToLabel)
    {
        copiedTerminalToLabel->insert(termToLabel);
    }
    copiedLabelToTerminal->reserve(labelToTerminal->size());
    //Labels for Terminals
    for (auto labelToTerm : *labelToTerminal)
    {
        copiedLabelToTerminal->insert(labelToTerm);
    }
    copiedRoots->reserve(roots->capacity());
    //Roots
    for (Node* root : *roots)
    {
        copiedRoots->push_back(root);
    }

    Forest newForest(copiedNodes, copiedTerminalToLabel, copiedLabelToTerminal, copiedRoots);

    return newForest;

}

}  //namespace graph
