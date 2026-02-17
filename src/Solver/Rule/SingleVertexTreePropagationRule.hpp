#ifndef PACE2026_SINGLE_VERTEX_TREE_PROPAGATION_RULE_HPP
#define PACE2026_SINGLE_VERTEX_TREE_PROPAGATION_RULE_HPP

#include "AbstractRule.hpp"
#include "../Action/DeleteEdgeAction.hpp"
#include <unordered_set>
#include <stack>

namespace solver
{

/// \brief If there is terminal which is a single vertex tree in one forest,
/// the corresponding terminals in the other forests can be safely transformed into single vertex trees as well.
///
/// \link https://gitlab.informatik.uni-bremen.de/pace-2026/orga/-/wikis/Branching/SingleVertexTreePropagationRule GitLab Doku
class SingleVertexTreePropagationRule : public AbstractRule
{
  protected:

    /// \brief set of all labels where the corresponding terminals
    /// are somewhere but not everywhere single vertex trees
    std::unordered_set<unsigned int> labelsToBeReduced;


    /// \brief Stack of action that modify the instance,
    /// filled in the apply method and unfilled in the unapply method
    std::stack<DeleteEdgeAction> changes = std::stack<DeleteEdgeAction>();;

public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    /// \param labelsToBeReduced the position where the rule can be applied,\n
    /// which is set of all labels where the corresponding terminals
    /// are somewhere but not everywhere single vertex trees
    SingleVertexTreePropagationRule(const std::shared_ptr<graph::Instance>& instance,
                                    const std::shared_ptr<Context>& context,
                                    const std::unordered_set<unsigned int>& labelsToBeReduced);

    /// \brief applies rule
    /// \see AbstractRule::apply
    /// \returns always return code  \c 0 (default, continue solving)
    int apply() override;

    void unapply() override;

    /// \brief It checks whether the SingleVertexTreePropagationRule is applicable and generates an instance of this rule if so.
    /// \returns shared_pointer to SingleVertexTreePropagationRule if rule is applicable, elso null pointer
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_SINGLE_VERTEX_TREE_PROPAGATION_RULE_HPP
