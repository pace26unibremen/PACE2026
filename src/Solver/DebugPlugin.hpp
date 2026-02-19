#ifndef PACE2026_DEBUG_PLUGIN_HPP
#define PACE2026_DEBUG_PLUGIN_HPP

#include "Rule/AbstractRule.hpp"

#include <fstream>
#include <stack>

namespace solver
{

/// \brief This is a plugin for the branching solver
/// that visualizes how the solver explores the search space as a dot graph.
/// It also writes all intermediate states of the instance as a dot graph within the calculation.
class DebugPlugin
{
    /// \brief stores ids for each state in the current branch
    std::stack<int> stateIDs = std::stack<int>();

    /// \brief maximum state id that was assigned
    int maxStateId = 0;

    /// \brief path to the directory where the debug information is dumped
    std::string dirPath;

    /// \brief out stream to the overview file
    std::ofstream overviewFile;

    /// \brief pointer instance that holds all inital forests
    std::shared_ptr<graph::Instance> shadowInstance;

    /// \brief pointer to instance
    std::shared_ptr<graph::Instance> instance;

    void writeStateNode();

    void writeRuleNode(const std::shared_ptr<AbstractRule>& rule);

    void dotInstance(const std::filesystem::path& path);

  public:
    /// \param _dirPath path to the directory where the debug information is dumped
    explicit DebugPlugin(std::string _dirPath);

    /// \brief call after every \c apply of a rule
    void onApply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief call after every \c unapply of a rule
    void onUnapply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief call when temporal changes are unapplied
    void onTempUnapply(const std::shared_ptr<solver::AbstractRule>& rule, bool lastRule);

    /// \brief call when temporal changes are applied
    void onTempApply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief call directly before end
    void onEnd();

    /// \brief init call
    void init(const std::shared_ptr<graph::Instance>& _instance);
};

}  // namespace solver

#endif  //PACE2026_DEBUG_PLUGIN_HPP
