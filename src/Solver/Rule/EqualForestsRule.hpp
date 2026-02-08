#ifndef PACE2026_SINGLE_FOREST_RULE_HPP
#define PACE2026_SINGLE_FOREST_RULE_HPP

#include "AbstractRule.hpp"

#include <unordered_set>

namespace solver
{

/// \brief Removes duplicate forests from instance.
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

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_SINGLE_FOREST_RULE_HPP
