#ifndef PACE2026_TREE_HPP
#define PACE2026_TREE_HPP

#include "Node.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace graph
{

/// \brief A representation of a tree.
class Tree
{
  private:
    /// \brief Vector that stores all nodes of the tree.
    std::shared_ptr<std::vector<Node>> nodes;

    /// \brief A map that stores all indices of terminals (in the \c nodes vector) with corresponding terminal labels.
    std::shared_ptr<std::unordered_map<int, unsigned int>> terminalIndexToLabel;

    /// \brief Index of root node in \c nodes vector.
    int rootIndex{};

  public:
    // ------------------------------------------------------------- //
    // ---- constructors ------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Constructor.
    Tree(std::shared_ptr<std::vector<Node>> nodes,
         std::shared_ptr<std::unordered_map<int, unsigned int>> terminalIndexToLabel,
         int rootIndex);

    /// \brief Constructor. Loads tree from a file in newick format.
    /// \param path to file
    [[maybe_unused]]
    explicit Tree(std::filesystem::path path);

    // ------------------------------------------------------------- //
    // ---- persistence -------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Writes tree to a stream in newick format.
    /// \param stream
    void write(std::ostream& out_file) const;

    /// \brief Writes tree to a stream in newick format.
    /// \param path to file
    [[maybe_unused]]
    void write(const std::string& path) const;

    /// \brief Writes tree as .dot graph to a stream.
    /// \param stream
    void dot(std::ostream& stream) const;

    /// \brief Writes tree to as .dot graph to a file.
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

    /// \brief Reference index of root node.
    /// \property
    [[nodiscard, maybe_unused]]
    int& RootIndex();

    /// \brief \c const reference index of root node.
    /// \property
    [[nodiscard, maybe_unused]]
    const int& RootIndex() const;

    // ------------------------------------------------------------- //
    // ---- graph manipulation ------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Removes an edge between a parent and a child from the tree.
    /// \param childIndex the index of the child node.
    /// \return the split sub tree.
    [[nodiscard]]
    std::shared_ptr<Tree> removeEdge(int childIndex);

    /// \brief Contracts a node.
    /// \note A node can be contracted iff\n
    /// 1. It isn't a terminal and\n
    /// 2. It has at most one child.
    /// \param nodeIndex the index of the node to contract.
    void contractNode(int nodeIndex);

    /// \brief Orders all siblings in the tree,
    /// such that for each node the subtree of the first child
    /// contains the minimum label.
    void orderSiblings();

    // ------------------------------------------------------------- //
    // ---- debug -------------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Writes tree as table to std::clog
    void print() const;

    /// \brief Checks whether the tree representation is valid
    /// and writes flaws to std::clog
    bool isValid() const;

    // ------------------------------------------------------------- //
    // ---- operators ---------------------------------------------- //
    // ------------------------------------------------------------- //

    /// \brief Equality operator that compares two trees for structural equality.
    /// \param other The tree to compare with.
    /// \return true if the trees are identical, false otherwise.
    bool operator==(const Tree& other) const;

};

}  // namespace graph

#endif  //PACE2026_TREE_HPP
