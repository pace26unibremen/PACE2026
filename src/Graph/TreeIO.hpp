#ifndef PACE2026_TREE_IO_HPP
#define PACE2026_TREE_IO_HPP

#include "Tree.hpp"
#include "filesystem"
#include "iostream"

namespace graph
{

/// \brief A static class for IO operations on trees.
class TreeIO
{
  public:
    static Tree ReadNewick(std::istream& stream);

    static void WriteNewick(const Tree& tree, std::ostream& stream);

    static void WriteDot(const Tree& tree, std::ostream& stream);
};

}  // namespace graph

#endif  //PACE2026_TREE_IO_HPP
