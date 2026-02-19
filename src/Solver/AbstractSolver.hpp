#ifndef PACE2026_ABSTRACT_SOLVER_HPP
#define PACE2026_ABSTRACT_SOLVER_HPP

#include "../Graph/Instance.hpp"
#include "../Graph/Forest.hpp"

namespace solver
{

/// \brief A base class for solver for the MAF problem
class AbstractSolver
{
  protected:
    /// \brief the instance to solve
    std::shared_ptr<graph::Instance> instance;

    /// \brief constructor
    /// \param instance to solve
    AbstractSolver(const std::shared_ptr<graph::Instance>& instance) : instance(instance) {};
  public:
    virtual ~AbstractSolver() = default;

    /// \brief solve the instance
    virtual std::shared_ptr<graph::Forest> solve() = 0;

    /// \brief \c const reference to instance
    /// \property Instance
    [[nodiscard, maybe_unused]]
    graph::Instance& Instance();

    /// \brief \c const reference to instance
    /// \property Instance
    [[nodiscard, maybe_unused]]
    const graph::Instance& Instance() const;
};

} // namespace solver

#endif  //PACE2026_ABSTRACT_SOLVER_HPP
