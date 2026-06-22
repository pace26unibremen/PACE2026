#ifndef PACE2026_SIBLING_RULE_FACTORY_HPP
#define PACE2026_SIBLING_RULE_FACTORY_HPP

#include "AbstractRule.hpp"

namespace solver
{

/// \brief This class provides combined `isApplicable` functions for the sibling rules.
class SiblingRuleFactory
{
  private:
    /// \brief Selects a sibling pair from the first forest of the instance.
    /// \param instance
    /// \param context
    /// \return The labels of a sibling pair
    static std::pair<unsigned int, unsigned int>
    getSiblings(const std::shared_ptr<graph::Instance>& instance,
                const std::shared_ptr<solver::Context>& context);

  public:
    /// \brief Checks whether \ref EqualPairReductionRule, \ref ACBranchingRule or \ref ABCBranchingRule
    /// can be applied and returns an instance of the best applicable rule,
    /// with the rules prioritized in the following order:
    /// 1. \ref EqualPairReductionRule
    /// 2. \ref ACBranchingRule
    /// 3. \ref ABCBranchingRule.
    /// If none of the rules is applicable the method returns a null pointer.
    /// \param instance on which the rule should be applied
    /// \param context contains additional information to the instance and the solver state
    /// \returns shared pointer to an instance of the best applicable rule
    static std::shared_ptr<AbstractRule> basicRules(const std::shared_ptr<graph::Instance>& instance,
                                                    const std::shared_ptr<solver::Context>& context);
};

}  //namespace solver

#endif  //PACE2026_SIBLING_RULE_FACTORY_HPP
