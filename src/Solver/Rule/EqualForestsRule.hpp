#ifndef PACE2026_SINGLE_FORESTRULE_HPP
#define PACE2026_SINGLE_FORESTRULE_HPP

#include "AbstractRule.hpp"

#include <unordered_set>

namespace solver
{

/// \brief Removes duplicate Forests from Instance
class EqualForestsRule : public AbstractRule
{
  protected:
    graph::Instance instanceBackUp;

    /// \brief Forests to be removed from instance
    std::unordered_set<std::shared_ptr<graph::Forest>> toBeRemoved;

  public:
    EqualForestsRule(const std::shared_ptr<graph::Instance>& instance,
                     const std::unordered_set<std::shared_ptr<graph::Forest>>& toBeRemoved);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_SINGLE_FORESTRULE_HPP
