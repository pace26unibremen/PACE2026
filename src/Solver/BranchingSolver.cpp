#include "BranchingSolver.hpp"

#include "Rule/SubtreeReductionRule.hpp"

#include <ranges>

solver::BranchingSolver::BranchingSolver(const std::shared_ptr<graph::Instance>& instance)
    : AbstractSolver(instance)
{
    this->context->branchingSolverConfiguration = this->configuration;
}

solver::BranchingSolver::BranchingSolver(const std::shared_ptr<graph::Instance>& instance,
                                         const std::shared_ptr<solver::BranchingSolverConfiguration>& configuration) :
    AbstractSolver(instance),
    configuration(configuration)
{
    this->context->branchingSolverConfiguration = this->configuration;
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
        for (const auto& plugin : configuration->plugins) plugin->onUnapply(rule);
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

        unapplyReductions();

        // notify plugins now that the instance is fully expanded (reductions undone)
        for (const auto& plugin : configuration->plugins) plugin->onNewBestSolution(context->bestSolutionSize);

        // write out the solution
        solution = std::make_shared<graph::Forest>(instance->at(0)->copy());

        // apply all reduction rules again to restore state of the solver
        for (const auto& reductionRule : appliedRules
            | std::views::filter([](const std::shared_ptr<AbstractRule>& r){ return r->IsReduction();}))
        {
            reductionRule->apply();
            for (const auto& plugin : configuration->plugins) plugin->onReductionReapply(reductionRule);
        }
    }
}

void solver::BranchingSolver::unapplyReductions()
{
    // The first reduction in appliedRules (forward) is the last one to be unapplied (reverse iteration).
    // Find it upfront so we can pass lastRule=true to plugins without a separate allocation.
    std::shared_ptr<AbstractRule> firstReduction;
    for (const auto& r : appliedRules)
        if (r->IsReduction()) { firstReduction = r; break; }

    // unapply all reduction rules to get solution for the original instance
    for (const auto& reductionRule : appliedRules | std::views::reverse
        | std::views::filter([](const std::shared_ptr<AbstractRule>& r){ return r->IsReduction();}))
    {
        reductionRule->unapply();
        for (const auto& plugin : configuration->plugins)
            plugin->onReductionUnapply(reductionRule, reductionRule == firstReduction);
    }
}

bool solver::BranchingSolver::solve()
{
    for (const auto& plugin : configuration->plugins) plugin->init(instance, context);

    // Try to apply the subtree reduction before starting the main solving process
    auto subtreeReduction = solver::SubtreeReductionRule::isApplicable(instance, context);
    if (subtreeReduction)
    {
        for (const auto& plugin : configuration->plugins) plugin->beforeApply(subtreeReduction);
        subtreeReduction->apply();
        for (const auto& plugin : configuration->plugins) plugin->onApply(subtreeReduction);
        appliedRules.push_back(subtreeReduction);
    }

    // apply rules repeatedly until a return is triggered
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

        for (const auto& plugin : configuration->plugins) plugin->beforeApply(rule);
        const auto returnCode = rule->apply();
        for (const auto& plugin : configuration->plugins) plugin->onApply(rule);
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
                    for (const auto& plugin : configuration->plugins) plugin->onBranchEnd();
                    for (const auto& plugin : configuration->plugins) plugin->onEnd();
                    return true;
                }
                else
                {
                    for (const auto& plugin : configuration->plugins) plugin->onBranchEnd();
                    checkSolutionCandidate();
                    calculationFinished = rollBackBranch();
                    break;
                }
            case RuleReturnCode::CutBranch:
                for (const auto& plugin : configuration->plugins) plugin->onBranchEnd();
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
                for (const auto& plugin : configuration->plugins) plugin->onEnd();
                *instance = {solution};
                return true;
            }
        }
    }
}
