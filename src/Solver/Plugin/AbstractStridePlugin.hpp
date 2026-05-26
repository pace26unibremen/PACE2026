#ifndef PACE2026_ABSTRACT_STRIDE_PLUGIN_HPP
#define PACE2026_ABSTRACT_STRIDE_PLUGIN_HPP

#include "AbstractPlugin.hpp"

#include <map>
#include <string>
#include <vector>

namespace solver::plugin
{

/// \brief Frozen snapshot of solver metrics taken at an improvement point.
///
/// Created by \ref ConvergencePlugin each time \c onNewBestSolution fires.
/// All numeric fields are cumulative from the start of the run.
struct Snapshot
{
    double wtime;                               ///< Wall-clock seconds elapsed since solving started
    std::size_t score;                          ///< Number of roots in the current best solution
    std::map<std::string, int> ruleCounts;      ///< Cumulative rule-application counts
    int branchOpens;                            ///< Cumulative branching-rule applications (internal nodes opened)
    int branchCloses;                           ///< Cumulative terminal branches reached (leaves closed)
    std::map<std::string, double> ruleTimes_ms; ///< Cumulative rule wall-clock times (ms)
};

/// \brief Base class for plugins that emit stride lines to stdout.
///
/// Extends \ref AbstractPlugin with:
/// - Three static \c toJson overloads for compact JSON serialisation
///   (hand-rolled, no external library).
/// - A static \c emitStrideLine helper that writes \c "#s key value\\n".
///
/// No new virtual hooks are introduced — subclasses still override only the
/// hooks they need from \ref AbstractPlugin.
class AbstractStridePlugin : public AbstractPlugin
{
  protected:
    /// \brief Serialise a string→int map to compact JSON, e.g. `{"r":3,"s":1}`.
    static std::string toJson(const std::map<std::string, int>& m);

    /// \brief Serialise a string→double map to compact JSON (3 decimal places), e.g. `{"r":1.234}`.
    static std::string toJson(const std::map<std::string, double>& m);

    /// \brief Serialise a vector of snapshots to a compact JSON array.
    static std::string toJson(const std::vector<Snapshot>& snapshots);

    /// \brief Write a single stride line \c "#s key jsonValue\\n" to \c std::cout.
    static void emitStrideLine(const std::string& key, const std::string& jsonValue);
};

} // namespace solver::plugin

#endif  //PACE2026_ABSTRACT_STRIDE_PLUGIN_HPP
