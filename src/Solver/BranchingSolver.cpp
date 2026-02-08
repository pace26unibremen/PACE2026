#include "BranchingSolver.hpp"

solver::BranchingSolver::BranchingSolver(const std::shared_ptr<graph::Instance>& instance)
    : AbstractSolver(instance)
{}

std::shared_ptr<solver::DebugPlugin>& solver::BranchingSolver::DebPlugin()
{
    return debPlugin;
}

std::vector<solver::isApplicableFn>& solver::BranchingSolver::ActiveRules()
{
    return activeRules;
}

bool solver::BranchingSolver::rollBackBranch()
{
    while (true)
    {
        if (changes.empty())
        {
            return true;
        }

        auto rule = changes.top();
        if (auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
        {
            if (branchingRule->isFullyExplored())
            {
                branchingRule->unapply();
                if (debPlugin) debPlugin->onUnapply(rule);

                changes.pop();
                if (not temporalChanges.empty() and temporalChanges.top() == branchingRule)
                {
                    temporalChanges.pop();
                }
            }
            else
            {
                branchingRule->unapply();
                if (debPlugin) debPlugin->onUnapply(rule);

                branchingRule->apply();
                if (debPlugin) debPlugin->onApply(rule);
                return false;
            }
        }
        else
        {
            rule->unapply();
            if (debPlugin) debPlugin->onUnapply(rule);
            changes.pop();
            if (not temporalChanges.empty() and temporalChanges.top() == rule)
            {
                temporalChanges.pop();
            }
        }
    }
}

void solver::BranchingSolver::checkSolutionCandidate()
{
    if (solution == nullptr or instance->at(0)->Roots().size() < solution->Roots().size())
    {
        context->bestSolutionSize = instance->at(0)->Roots().size();
        std::stack<std::shared_ptr<AbstractRule>> temporalChangesCopy = std::stack<std::shared_ptr<AbstractRule>>();
        while (not temporalChanges.empty())
        {
            temporalChanges.top()->unapply();
            temporalChangesCopy.push(temporalChanges.top());
            temporalChanges.pop();
            if (debPlugin) debPlugin->onTempUnapply(temporalChangesCopy.top(), temporalChanges.empty());
        }
        solution = std::make_shared<graph::Forest>(instance->at(0)->copy());
        while (not temporalChangesCopy.empty())
        {
            temporalChangesCopy.top()->apply();
            if (debPlugin) debPlugin->onTempApply(temporalChangesCopy.top());
            temporalChanges.push(temporalChangesCopy.top());
            temporalChangesCopy.pop();
        }
    }
}

std::shared_ptr<graph::Forest> solver::BranchingSolver::solve()
{
    if (debPlugin) debPlugin->init(instance);

    // apply rules repeatedly until a return is triggerd
    while (true)
    {
        // check the rules for applicability
        for (const auto& isApplicable : activeRules)
        {
            auto rule = isApplicable(instance, context);

            // if rule was not applicable, try next
            if (not rule) continue;

            const auto returnCode = rule->apply();
            if (debPlugin) debPlugin->onApply(rule);

            changes.push(rule);
            if (std::dynamic_pointer_cast<PairEqualRule>(rule))
            {
                temporalChanges.push(rule);
            }

            bool calculationFinished = false;
            switch (returnCode)
            {
                case 0: // default continue calculation
                    break;
                case -1: // imidate return
                    calculationFinished = true;
                    break;
                case 1: // found a solution candidate
                    checkSolutionCandidate();
                    calculationFinished = rollBackBranch();
                    break;
                case 2: // cut branch
                    calculationFinished = rollBackBranch();
                    break;
                default:
                    throw std::logic_error("BranchingSolver : solve : undefined return code rule " + rule->name());
            }

            if (calculationFinished)
            {
                if (debPlugin) debPlugin->onEnd();
                return solution;
            }
            break;
        }
    }
}
