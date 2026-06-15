#include "ReductionSolver.hpp"

solver::ReductionSolver::ReductionSolver(const std::shared_ptr<graph::Instance>& instance) :
        AbstractSolver(instance)
{}

solver::ReductionSolver::ReductionSolver(const std::shared_ptr<graph::Instance>& instance,
                                         const std::shared_ptr<solver::SolverConfiguration>& configuration) :
        AbstractSolver(instance),
        configuration(configuration)
{}

bool solver::ReductionSolver::solve()
{
    if (configuration->subtreeReduction)
    {
        subtreeReductionRule = solver::SubtreeReductionRule::isApplicable(instance, context);
        if (subtreeReductionRule)
        {
            subtreeReductionRule->apply();
        }
    }
    return false;
}

void solver::ReductionSolver::unapplyReductions()
{
    if (configuration->subtreeReduction and subtreeReductionRule)
    {
        subtreeReductionRule->unapply();
    }
}
