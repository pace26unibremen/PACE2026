#ifndef PACE2026_FOREST_HPP
#define PACE2026_FOREST_HPP

#include "Node.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace graph
{

/// \brief A representation of a forest.
class Forest
{
  private:
    /// \brief Vector that stores all nodes of the forest.
    std::shared_ptr<std::vector<Node>> nodes;

    /// \brief A map that stores all indices of terminals (in the \c nodes vector) with corresponding terminal labels.
    std::shared_ptr<std::unordered_map<int, unsigned int>> terminalIndexToLabel;

    /// \brief A map that stores all leaf labels with corresponding terminal indices (in the \c nodes vector).
    std::shared_ptr<std::unordered_map<unsigned int, int>> labelToTerminalIndex;

    /// \brief Indices of root nodes in \c nodes vector.
    std::shared_ptr<std::vector<int>> rootIndices;

  public:
    // ------------------------------------------------------------- //
    // ---- constructors ------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Constructor.
    Forest(std::shared_ptr<std::vector<Node>> nodes,
           std::shared_ptr<std::unordered_map<int, unsigned int>> terminalIndexToLabel,
           std::shared_ptr<std::unordered_map<unsigned int, int>> labelToTerminalIndex,
           std::shared_ptr<std::vector<int>> rootIndices);

    /// \brief Constructor. Loads forest from a file in newick format.
    /// \param path to file
    /// \param numberOfTerminals number of leafs (for optimized memory allocation only). \c 0 if unknown.
    /// \param numberOfTrees number of trees. \c 1 is default.
    [[maybe_unused]]
    explicit Forest(const std::filesystem::path& path, int numberOfTerminals = 0, int numberOfTrees = 1);

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
    std::unordered_map<int, unsigned int>& Terminals();

    /// \brief \c const reference to terminals map
    /// \property
    /// \first Terminal indices.
    /// \second Corresponding terminal label.
    [[nodiscard, maybe_unused]]
    const std::unordered_map<int, unsigned int>& Terminals() const;

    /// \brief Reference to label-to-node-index map
    /// \property
    /// \first Leaf labels.
    /// \second Corresponding node index.
    [[nodiscard, maybe_unused]]
    std::unordered_map<unsigned int, int>& LabelToTerminalIndex();

    /// \brief \c const reference to label-to-node-index map
    /// \property
    /// \first Leaf labels.
    /// \second Corresponding node index.
    [[nodiscard, maybe_unused]]
    const std::unordered_map<unsigned int, int>& LabelToTerminalIndex() const;

    /// \brief Reference index of root node.
    /// \property
    [[nodiscard, maybe_unused]]
    std::vector<int>& RootIndices();

    /// \brief \c const reference index of root node.
    /// \property
    [[nodiscard, maybe_unused]]
    const std::vector<int>& RootIndices() const;

    // ------------------------------------------------------------- //
    // ---- graph manipulation ------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Removes an edge between a parent and a child.
    /// \param childIndex the index of the child node.
    void removeEdge(int childIndex);

    /// \brief \b 1. Sorts the children of each node,
    /// such that the first child
    /// contains the minimum label of both children.\n
    /// And \b 2. fills \c subtreeTerminals for each node.
    void sortChildrenAndCollectTerminals();

    /// \brief Returns the maximum common X-Forest of this forest and the input forest.
    Forest maximumCommonSubforest(Forest& other);

    // ------------------------------------------------------------- //
    // ---- debug -------------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Writes forest as table to std::clog
    void print() const;

    /// \brief Checks whether the forest representation is valid
    /// and writes flaws to std::clog
    bool isValid() const;

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
