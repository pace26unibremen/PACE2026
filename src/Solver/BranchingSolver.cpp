#include "BranchingSolver.hpp"

#include <algorithm>
#include <ranges>

solver::BranchingSolver::BranchingSolver(const std::shared_ptr<graph::Instance>& instance)
    : AbstractSolver(instance)
{
    this->context->solverConfiguration = this->configuration;
}

solver::BranchingSolver::BranchingSolver(const std::shared_ptr<graph::Instance>& instance,
                                         const std::shared_ptr<solver::SolverConfiguration>& configuration) :
    AbstractSolver(instance),
    configuration(configuration)
{
    this->context->solverConfiguration = this->configuration;
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
        if (configuration->debPlugin) configuration->debPlugin->onUnapply(rule);
        appliedRules.pop_back();

        if (auto branchingRule = std::dynamic_pointer_cast<AbstractBranchingRule>(rule))
        {
            if (not branchingRule->isFullyExplored())
            {
                // to enter the next branch, we have to apply the branching rule again
                applyNext.emplace(branchingRule);
                return false;
            }
        }
    }
}

void solver::BranchingSolver::checkSolutionCandidate()
{
    auto candidateWeight = context->weightFunction(instance->at(0));
    if (context->bestSolutionWeight > candidateWeight)
    {
        context->bestSolutionWeight = candidateWeight;
        auto branchCloneView = appliedRules | std::views::transform(
            [](const std::shared_ptr<AbstractRule>& r) { return r->clone(); });
        solutionBranch = {branchCloneView.begin(), branchCloneView.end()};
    }
}

void solver::BranchingSolver::unapplyReductions()
{
    // unapply all reduction rules to get solution for the original instance
    for (const auto& reductionRule : appliedRules | std::views::reverse
        | std::views::filter([](const std::shared_ptr<AbstractRule>& r){ return r->IsReduction();}))
    {
        reductionRule->unapply();
        if (configuration->debPlugin) configuration->debPlugin->onTempUnapply(reductionRule, false);
    }
}

bool solver::BranchingSolver::solve()
{
    if (configuration->debPlugin) configuration->debPlugin->init(instance);

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
            for (const auto& isApplicable : configuration->activeRules)
            {
                rule = isApplicable(instance, context);
                // take first applicable rule
                if (rule) break;
            }
        }

        const auto returnCode = rule->apply();
        if (configuration->debPlugin) configuration->debPlugin->onApply(rule);
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
                if (configuration->boundedDephtSearch)
                {
                    return true;
                }
                else
                {
                    checkSolutionCandidate();
                    calculationFinished = rollBackBranch();
                    break;
                }
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
            if (configuration->boundedDephtSearch)
            {
                context->maxSolutionSize++;
            }
            else
            {
                // apply solution branch
                for (const auto& r : solutionBranch)
                {
                    appliedRules.push_back(r);
                    r->apply();
                }
                if (configuration->debPlugin) configuration->debPlugin->onEnd();
                return true;
            }
        }
    }
}
