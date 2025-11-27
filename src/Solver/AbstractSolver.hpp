#ifndef PACE2026_ABSTRACT_SOLVER_HPP
#define PACE2026_ABSTRACT_SOLVER_HPP

#include "../Graph/Instance.hpp"
#include "../Graph/Forest.hpp"

namespace solver
{

class AbstractSolver
{
  protected:
    /// \brief the instance to solve
    graph::Instance instance;
  public:
    /// \brief solve the instance
    virtual graph::Forest solve() = 0;

    /// \brief \c const reference to instance
    [[nodiscard, maybe_unused]]
    graph::Instance& Instance();

    /// \brief \c const reference to instance
    [[nodiscard, maybe_unused]]
    const graph::Instance& Instance() const;
};

} // namespace solver

#endif  //PACE2026_ABSTRACT_SOLVER_HPP
