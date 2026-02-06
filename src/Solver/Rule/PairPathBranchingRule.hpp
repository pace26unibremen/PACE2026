#ifndef PACE2026_SIBLING_PATH_BRANCHING_RULE_HPP
#define PACE2026_SIBLING_PATH_BRANCHING_RULE_HPP

#include "../Action/DeleteEdgeAction.hpp"
#include "AbstractBranchingRule.hpp"

#include <list>
#include <stack>

namespace solver
{

/// \brief If there is a pair of two terminals in one forest and in some other forests of the instance
/// the corresponding terminals aren't siblings (but are connected), we can branch into three cases:\n
/// 1. The first terminal is a single vertex tree in the solution. -> cut this terminal
/// 2. The other terminal is a single vertex tree in the solution. -> cut the other terminal
/// 3. The two terminals are a pair in the solution. -> cut all subtrees on the path between both terminals
class PairPathBranchingRule : public AbstractBranchingRule
{
  protected:

    /// \brief first label
    unsigned int label1;

    /// \brief second label
    unsigned int label2;

    /// \brief mapping for each forest to the list of nodes that can be cutted
    /// (in the corresponding branch, where the both terminals are siblings).
    std::unordered_map<std::shared_ptr<graph::Forest>, std::list<graph::Node*>> forestToPathDeletions;

    /// \brief Stack of action that modify the instance,
    /// filled in the apply method and unfilled in the unapply method
    std::stack<DeleteEdgeAction> changes = std::stack<DeleteEdgeAction>();

  public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    /// \param cuts the position where the rule can be applied,\n
    /// which is a tuple of the two relevant terminal labels and a mapping for each forest to the list of nodes
    /// that should be cutted, to make the both terminals siblings.
    PairPathBranchingRule(
        const std::shared_ptr<graph::Instance>& instance,
        const std::shared_ptr<Context>& context,
        const std::tuple<unsigned int, unsigned int,
                         std::unordered_map<std::shared_ptr<graph::Forest>, std::list<graph::Node*>>>& cuts);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_SIBLING_PATH_BRANCHING_RULE_HPP
