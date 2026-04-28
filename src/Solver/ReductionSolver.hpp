#ifndef PACE2026_REDUCTIONSOLVER_HPP
#define PACE2026_REDUCTIONSOLVER_HPP

#include "AbstractSolver.hpp"
#include "Rule/ClusterReductionRule.hpp"
#include "Rule/SubtreeReductionRule.hpp"
#include "SolverConfiguration.hpp"

namespace solver
{

class ReductionSolver : public AbstractSolver
{
private:
    std::shared_ptr<solver::SolverConfiguration> configuration;
    std::shared_ptr<solver::AbstractRule> clusterReductionRule = nullptr;
    std::shared_ptr<solver::AbstractRule> subtreeReductionRule = nullptr;
    std::shared_ptr<solver::Context> context = std::make_shared<solver::Context>();
public:
    /// \brief Constructor for a reduction solver.
    /// \param instance to solve
    explicit ReductionSolver(const std::shared_ptr<graph::Instance>& instance);

    /// \brief Constructor for a reduction solver.
    /// \param instance to solve
    /// \param configuration for the solver
    explicit ReductionSolver(const std::shared_ptr<graph::Instance>& instance,
                             const std::shared_ptr<solver::SolverConfiguration>& configuration);

    /// \brief solve the instance
    virtual bool solve() override;

    /// \brief Unapplies all reduction rules, that where applied to the instance.
    virtual void unapplyReductions() override;

};


} // namespave solver

#endif //PACE2026_REDUCTIONSOLVER_HPP
