#ifndef PACE2026_SINGLE_FOREST_RULE_HPP
#define PACE2026_SINGLE_FOREST_RULE_HPP

#include "AbstractRule.hpp"

#include <unordered_set>

namespace solver
{

/// \brief Removes duplicate forests from instance.
///
/// \link https://gitlab.informatik.uni-bremen.de/pace-2026/orga/-/wikis/Branching/EqualForestsRule GitLab Docu
class EqualForestsRule : public AbstractRule
{
  protected:
    graph::Instance instanceBackUp;

    /// \brief Forests to be removed from instance
    std::unordered_set<std::shared_ptr<graph::Forest>> toBeRemoved;

  public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    /// \param toBeRemoved the position where the rule can be applied,\n
    /// which is set of all forests that can be removed.
    EqualForestsRule(const std::shared_ptr<graph::Instance>& instance,
                     const std::shared_ptr<Context>& context,
                     const std::unordered_set<std::shared_ptr<graph::Forest>>& toBeRemoved);

    /// \brief applies rule
    /// \see AbstractRule::apply
    /// \returns two return codes are possible:
    /// - return code \c 1 (rule solves the instance), if instance now contains only one forests
    /// - return code \c 0 (default, continue solving), if instance still contains multiple forests
    int apply() override;

    void unapply() override;

    /// \brief It checks whether the EqualForestsRule is applicable and generates an instance of this rule if so.
    /// \returns shared_pointer to EqualForestsRule if rule is applicable, elso null pointer
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_SINGLE_FOREST_RULE_HPP
