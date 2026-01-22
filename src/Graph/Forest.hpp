#ifndef PACE2026_FOREST_HPP
#define PACE2026_FOREST_HPP

#include "Node.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <set>
#ifdef DEBUG_IMAGE_VIEW_GRAPH
#include <opencv2/core.hpp>
#endif

namespace graph
{

/// \brief A representation of a forest.
class Forest
{
  private:
    /// \brief Vector that stores all nodes of the forest.
    std::shared_ptr<std::vector<Node>> nodes;

    /// \brief A map that stores all indices of terminals (in the \c nodes vector) with corresponding terminal labels.
    std::shared_ptr<std::unordered_map<Node*, unsigned int>> terminalToLabel;

    /// \brief A map that stores all leaf labels with corresponding terminal indices (in the \c nodes vector).
    std::shared_ptr<std::unordered_map<unsigned int, Node*>> labelToTerminal;

    /// \brief Indices of root nodes in \c nodes vector.
    std::shared_ptr<std::vector<Node*>> roots;

    /// \brief \b 1. Sorts the children of each node,
    /// such that the left child
    /// contains the minimum label of both children.\n
    /// And \b 2. fills \c subtreeTerminals for each node.
    void sortChildrenAndCollectTerminals();

  public:
    #ifdef DEBUG_IMAGE_VIEW_GRAPH
    // an image of the forest for debug purpose
    cv::Mat image;
    void renderImage();
    #endif
    // ------------------------------------------------------------- //
    // ---- constructors ------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Constructor.
    Forest(std::shared_ptr<std::vector<Node>> nodes,
           std::shared_ptr<std::unordered_map<Node*, unsigned int>> terminalToLabel,
           std::shared_ptr<std::unordered_map<unsigned int, Node*>> labelToTerminal,
           std::shared_ptr<std::vector<Node*>> roots);

    /// \brief Constructor. Loads forest from a file in newick format.
    /// \param path to file
    /// \param numberOfTerminals number of leafs.
    /// \param numberOfTrees number of trees.
    [[maybe_unused]]
    explicit Forest(const std::filesystem::path& path, int numberOfTerminals, int numberOfTrees);

    // ------------------------------------------------------------- //
    // ---- persistence -------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Writes forest to a stream in newick format.
    /// \param stream
    void write(std::ostream& out_file) const;

    /// \brief Writes forest to a stream in newick format.
    /// \param path to file
    [[maybe_unused]]
    void write(const std::string& path) const;

    /// \brief Writes forest as .dot graph to a stream.
    /// \param stream
    void dot(std::ostream& stream) const;

    /// \brief Writes forest to as .dot graph to a file.
    /// \param path to file
    [[maybe_unused]]
    void dot(const std::string& path) const;

    // ------------------------------------------------------------- //
    // ---- access to member fields -------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Reference to nodes vector.
    /// \property
    [[nodiscard, maybe_unused]]
    std::vector<Node>& Nodes();

    /// \brief \c const reference to nodes vector.
    /// \property
    [[nodiscard, maybe_unused]]
    const std::vector<Node>& Nodes() const;

    /// \brief Reference to terminals map
    /// \property
    /// \first Terminal indices.
    /// \second Corresponding terminal label.
    [[nodiscard, maybe_unused]]
    std::unordered_map<Node*, unsigned int>& Terminals();

    /// \brief \c const reference to terminals map
    /// \property
    /// \first Terminal indices.
    /// \second Corresponding terminal label.
    [[nodiscard, maybe_unused]]
    const std::unordered_map<Node*, unsigned int>& Terminals() const;

    /// \brief Reference to label-to-node-index map
    /// \property
    /// \first Leaf labels.
    /// \second Corresponding node index.
    [[nodiscard, maybe_unused]]
    std::unordered_map<unsigned int, Node*>& LabelToTerminal();

    /// \brief \c const reference to label-to-node-index map
    /// \property
    /// \first Leaf labels.
    /// \second Corresponding node index.
    [[nodiscard, maybe_unused]]
    const std::unordered_map<unsigned int, Node*>& LabelToTerminal() const;

    /// \brief Reference index of root node.
    /// \property
    [[nodiscard, maybe_unused]]
    std::vector<Node*>& Roots();

    /// \brief \c const reference index of root node.
    /// \property
    [[nodiscard, maybe_unused]]
    const std::vector<Node*>& Roots() const;

    /// \brief returns the index of the root node in the nodes vector, that has \c node in its subtree
    [[nodiscard, maybe_unused]]
    Node* rootOf(const Node& node) const;

    /// \brief returns the index of the root node in the nodes vector, that has \c node in its subtree
    [[nodiscard, maybe_unused]]
    Node* rootOf(Node* node) const;

    /// \brief Returns the maximum common X-Forest of this forest and the input forest.
    std::vector<Node*> maximumCommonSubforestRoots(const Forest& other);

    // ------------------------------------------------------------- //
    // ---- debug -------------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Writes forest as table to std::clog
    void print() const;

    /// \brief Checks whether the forest representation is valid
    /// and writes flaws to std::clog
    bool isValid() const;

    /// \brief Operator that checks if the first tree is a subtree of the second.
    /// \param other The tree to compare with.
    /// \return true if the first tree is a subtree, false otherwise.
    bool isTrueSubtreeOf(const Forest& other) const;

    bool hasIdenticalSubtree(const Forest& other, Node* thisNode, Node* otherNode);

    /// \brief Checks relations between parent and children. Goes deeper recursively.
    bool checkTriple(Node* parentIndex, std::unordered_map<Node*, unsigned int>& leafs, std::set<Node*>& pointers,
                     unsigned int& lastSmallestTerminal) const;

    // ------------------------------------------------------------- //
    // ---- operators ---------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Equality operator that compares two trees for structural equality.
    /// \param other The tree to compare with.
    /// \return true if the trees are identical, false otherwise.
    bool operator==(const Forest& other) const;




    // ------------------------------------------------------------- //
    // ---- copy func ---------------------------------------------- //
    // ------------------------------------------------------------- //
    /// \brief Tree copy function that makes a separate copy to the copied tree.
    /// @return A Copy of the Tree
    Forest copy();

};


}  // namespace graph

#endif  //PACE2026_FOREST_HPP
