#ifndef PACE2026_PAIR_UNCONNECTED_BRANCHING_RULE_HPP
#define PACE2026_PAIR_UNCONNECTED_BRANCHING_RULE_HPP

#include "AbstractBranchingRule.hpp"
#include "../Action/DeleteEdgeAction.hpp"

#include <list>
#include <stack>

namespace solver
{

/// \brief If there is a pair of two terminals in one forest and in some other forests of the instance
/// the corresponding terminals aren't connected (are in different trees), we can branch into two cases:\n
/// 1. The first terminal is a single vertex tree in the solution. -> cut this terminal
/// 2. The other terminal is a single vertex tree in the solution. -> cut the other terminal
class PairUnconnectedBranchingRule : public AbstractBranchingRule
{
  protected:

    /// \brief first label
    unsigned int label1;

    /// \brief second label
    unsigned int label2;

    /// list of all forests where the two terminals  aren't siblings.
    std::list<std::shared_ptr<graph::Forest>> forestsConnectedLabels;

    /// \brief Stack of action that modify the instance,
    /// filled in the apply method and unfilled in the unapply method
    std::stack<DeleteEdgeAction> changes = std::stack<solver::DeleteEdgeAction>();

  public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    /// \param affectedForests the position where the rule can be applied,\n
    /// which is a tuple of the two relevant terminal labels and a list of all forests where the two terminals
    /// aren't siblings.
    PairUnconnectedBranchingRule(
        const std::shared_ptr<graph::Instance>& instance,
        const std::shared_ptr<Context>& context,
        const std::tuple<unsigned int, unsigned int, std::list<std::shared_ptr<graph::Forest>>>& affectedForests);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_PAIR_UNCONNECTED_BRANCHING_RULE_HPP
