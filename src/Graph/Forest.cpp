
#include "Forest.hpp"

#include "ForestIO.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_set>
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

    for(int nodeIndex = 0; nodeIndex < (int) nodes->size(); nodeIndex++)
    {
        const Node& node = nodes->at(nodeIndex);

        if(node.parentIndex == -1 and
           node.leftChildIndex == -1 and
           node.rightChildIndex == -1)
        {
            // fully reduced node or single node tree
            continue;
        }

        // root or leaf
        if(node.leftChildIndex >= 0 and node.leftChildIndex < (int) nodes->size() and
           node.rightChildIndex >= 0 and node.rightChildIndex < (int) nodes->size())
        {
            const Node& fstChild = nodes->at(node.leftChildIndex);
            const Node& sndChild = nodes->at(node.rightChildIndex);

            // node.child.parent == node
            if(fstChild.parentIndex != nodeIndex)
            {
                std::clog << "Forest: isValid: inconstant parent-child relation:\n"
                             "   parent (" << nodeIndex << ") -> (" << node.leftChildIndex <<") child\n" <<
                             "   parent (" << fstChild.parentIndex << ") <- (" << node.leftChildIndex <<") child\n";
                valid = false;
            }
            if(sndChild.parentIndex != nodeIndex)
            {
                std::clog << "Forest: isValid: inconstant parent-child relation:\n"
                             "   parent (" << nodeIndex << ") -> (" << node.rightChildIndex <<") child\n" <<
                             "   parent (" << sndChild.parentIndex << ") <- (" << node.rightChildIndex <<") child\n";
                valid = false;
            }

            // TODO check sibling order and label structure
        }
        // node is terminal
        else if (node.leftChildIndex == -1 and node.rightChildIndex == -1)
        {
            // TODO check is terminal
        }
        else
        {
            std::clog << "Forest: isValid: invalid sibling indices:\n"
                         "   node (" << nodeIndex << "): fst Child (" << node.leftChildIndex <<") "
                        "snd Child (" << node.rightChildIndex <<") \n";
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
