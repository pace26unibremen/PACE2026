#include "BranchingSolver.hpp"

#include "Rule/EqualForestsRule.hpp"
#include "Rule/PairEqualRule.hpp"
#include "Rule/PairPathBranchingRule.hpp"
#include "Rule/PairUnconnectedBranchingRule.hpp"
#include "Rule/SingleVertexTreePropagationRule.hpp"
#include "Rule/DebugAssertFalseRule.hpp"

#include <functional>

solver::BranchingSolver::BranchingSolver(const std::shared_ptr<graph::Instance>& instance)
    : AbstractSolver(instance)
{}

std::shared_ptr<graph::Forest> solver::BranchingSolver::solve()
{
    std::vector<std::function<std::shared_ptr<AbstractRule>(std::shared_ptr<graph::Instance> instance)>>
        applicableCheck = {solver::EqualForestsRule::isApplicable,
                           solver::SingleVertexTreePropagationRule::isApplicable,
                           solver::PairUnconnectedBranchingRule::isApplicable,
                           solver::PairEqualRule::isApplicable,
                           solver::PairPathBranchingRule::isApplicable,
                           solver::DebugAssertFalseRule::isApplicable};

    std::shared_ptr<graph::Forest> solution = nullptr;

    while (true)
    {
        if (instance->size() == 1)
        {
            if (solution == nullptr or instance->at(0)->RootIndices().size() < solution->RootIndices().size())
            {
                std::stack<std::shared_ptr<AbstractRule>> temporalChangesCopy = std::stack<std::shared_ptr<AbstractRule>>();
                while (not temporalChanges.empty())
                {
                    temporalChanges.top()->unapply();
                    temporalChangesCopy.push(temporalChanges.top());
                    temporalChanges.pop();
                }
                solution = std::make_shared<graph::Forest>((*instance->at(0)).copy());
                while (not temporalChangesCopy.empty())
                {
                    temporalChangesCopy.top()->apply();
                    temporalChanges.push(temporalChangesCopy.top());
                    temporalChangesCopy.pop();
                }
            }
            while (true)
            {
                if (changes.empty())
                {
                    return solution;
                }
                auto rule = changes.top();
                if (auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
                {
                    if (branchingRule->isFullyExplored())
                    {
                        branchingRule->unapply();
                        changes.pop();
                        if (not temporalChanges.empty() and temporalChanges.top() == branchingRule)
                        {
                            temporalChanges.pop();
                        }
                    }
                    else
                    {
                        branchingRule->unapply();
                        branchingRule->apply();
                        break;
                    }
                }
                else
                {
                    rule->unapply();
                    changes.pop();
                    if (not temporalChanges.empty() and temporalChanges.top() == rule)
                    {
                        temporalChanges.pop();
                    }
                }
            }
        }

        for (const auto& isApplicable : applicableCheck)
        {
            auto rule = isApplicable(instance);
            if (rule != nullptr)
            {
                rule->apply();
                changes.push(rule);
                if (std::dynamic_pointer_cast<PairEqualRule>(rule) != nullptr)
                {
                    temporalChanges.push(rule);
                }
                break;
            }
        }
    }
}
