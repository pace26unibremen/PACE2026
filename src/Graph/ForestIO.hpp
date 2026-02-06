#ifndef PACE2026_FOREST_IO_HPP
#define PACE2026_FOREST_IO_HPP

#include "Forest.hpp"
#include "filesystem"
#include "iostream"

namespace graph
{

/// \brief A static class for IO operations on forests.
class ForestIO
{
  public:
    /// Reads a forest from input stream
    /// \param stream the input stream
    /// \param numberOfTerminals the number of leafs, set this value for optimal memory allocation
    /// \param numberOfTrees the number of trees in the forest, default is 1
    /// \return a forest
    static Forest ReadNewick(std::istream& stream, int numberOfTerminals = 0, int numberOfTrees = 1);

    static void WriteNewick(const Forest& tree, std::ostream& stream);

    static void WriteDot(const Forest& tree, std::ostream& stream);

    static void WriteDotSubgraph(const Forest& tree, std::ostream& stream, std::string subgraphParams = "");
};

}  // namespace graph

#endif  //PACE2026_FOREST_IO_HPP
