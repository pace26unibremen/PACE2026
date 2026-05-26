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
    /// \brief Convert a CamelCase name to lowercase snake_case.
    ///
    /// Inserts an underscore before each uppercase letter (except the first) and
    /// lowercases the entire string.  This is used to produce stable, readable JSON
    /// keys from \c AbstractRule::name() return values while keeping the C++ class
    /// names CamelCase throughout the hot solver path.
    ///
    /// Examples: \c "CutBranchRule" → \c "cut_branch_rule",
    ///           \c "PairPathBranchingRule" → \c "pair_path_branching_rule".
    static std::string toSnakeCase(const std::string& camelCase);

    /// \brief Serialise a string→int map to compact JSON, e.g. `{"r":3,"s":1}`.
    static std::string toJson(const std::map<std::string, int>& m);

    /// \brief Serialise a string→double map to compact JSON (3 decimal places), e.g. `{"r":1.234}`.
    static std::string toJson(const std::map<std::string, double>& m);

    /// \brief Serialise a vector of snapshots to a compact JSON array.
    /// Rule name keys in each snapshot are converted to snake_case at this point.
    static std::string toJson(const std::vector<Snapshot>& snapshots);

    /// \brief Like toJson(map<string,int>) but converts each key via toSnakeCase first.
    /// Use this at emit time (onEnd) to avoid repeated allocation during the hot solver path.
    static std::string toJsonSnakeKeys(const std::map<std::string, int>& m);

    /// \brief Like toJson(map<string,double>) but converts each key via toSnakeCase first.
    /// Use this at emit time (onEnd) to avoid repeated allocation during the hot solver path.
    static std::string toJsonSnakeKeys(const std::map<std::string, double>& m);

    /// \brief Write a single stride line \c "#s key jsonValue\\n" to \c std::cout.
    static void emitStrideLine(const std::string& key, const std::string& jsonValue);
};

} // namespace solver::plugin

#endif  //PACE2026_ABSTRACT_STRIDE_PLUGIN_HPP
