#include "BranchingSolver.hpp"

#include "Rule/SubtreeReductionRule.hpp"

#include <ranges>

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
    // all rule suggestions can be discarded at the end of a branch
    applyNext = std::queue<std::shared_ptr<AbstractRule>>();

    while (true)
    {
        if (appliedRules.empty())
        {
            return true;
        }

        auto rule = appliedRules.back();
        rule->unapply();
        if (debPlugin) debPlugin->onUnapply(rule);
        appliedRules.pop_back();

        if (auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
        {
            if (not branchingRule->isFullyExplored())
            {
                // to enter the next branch we have to apply the branching rule again
                applyNext.emplace(branchingRule);
                return false;
            }
        }
    }
}

void solver::BranchingSolver::checkSolutionCandidate()
{
    if (solution == nullptr or instance->at(0)->Roots().size() < solution->Roots().size())
    {
        // update context (we have a better solution)
        context->bestSolutionSize = instance->at(0)->Roots().size();

        // unapply all reduction rules to get solution for the original instance
        for (const auto& reductionRule : appliedRules | std::views::reverse
            | std::views::filter([](const std::shared_ptr<AbstractRule>& r){ return r->IsReduction();}))
        {
            reductionRule->unapply();
            if (debPlugin) debPlugin->onTempUnapply(reductionRule, false);
        }

        // write out the solution
        solution = std::make_shared<graph::Forest>(instance->at(0)->copy());

        // un-unapply all reduction rules again to restore state of the solver
        for (const auto& reductionRule : appliedRules
            | std::views::filter([](const std::shared_ptr<AbstractRule>& r){ return r->IsReduction();}))
        {
            reductionRule->apply();
            if (debPlugin) debPlugin->onTempApply(reductionRule);
        }
    }
}

std::shared_ptr<graph::Forest> solver::BranchingSolver::solve()
{
    if (debPlugin) debPlugin->init(instance);

    // Try to apply the subtree reduction before starting the main solving process
    auto subtreeReduction = solver::SubtreeReductionRule::isApplicable(instance, context);
    if (subtreeReduction)
    {
        subtreeReduction->apply();
        if (debPlugin) debPlugin->onApply(subtreeReduction);
        appliedRules.push_back(subtreeReduction);
    }

    // apply rules repeatedly until a return is triggerd
    while (true)
    {
        std::shared_ptr<AbstractRule> rule = nullptr;

        // check if we have rules in the pipeline
        if (not applyNext.empty())
        {
            // take first rule of queue
            rule = applyNext.front();
            applyNext.pop();
        }
        else
        {
            // check the rules for applicability
            for (const auto& isApplicable : activeRules)
            {
                rule = isApplicable(instance, context);
                // take first applicable rule
                if (rule) break;
            }
        }

        const auto returnCode = rule->apply();
        if (debPlugin) debPlugin->onApply(rule);
        appliedRules.push_back(rule);

        bool calculationFinished = false;
        switch (returnCode)
        {
            case RuleReturnCode::Continue:
                break;
            case RuleReturnCode::ContinueWithRuleSuggestion:
                for (const auto& r : *rule->NextRuleSuggestion())
                {
                    applyNext.emplace(r);
                }
                break;
            case RuleReturnCode::EndBranchWithSolutionCandidate:
                checkSolutionCandidate();
                calculationFinished = rollBackBranch();
                break;
            case RuleReturnCode::CutBranch:
                calculationFinished = rollBackBranch();
                break;
            case RuleReturnCode::ImidateReturn:
                calculationFinished = true;
                break;
            default:
                throw std::logic_error("BranchingSolver : solve : undefined return code rule " + rule->name());
        }

        if (calculationFinished)
        {
            if (debPlugin) debPlugin->onEnd();
            return solution;
        }
    }
}
