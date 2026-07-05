#ifndef PACE2026_DUAL_LOWER_BOUND_HPP
#define PACE2026_DUAL_LOWER_BOUND_HPP

#include "../Graph/Instance.hpp"

namespace solver
{

/// \brief A certified lower bound L on the MAF optimum k* (in component units): every maximum
/// agreement forest has at least L components. It is derived from a feasible dual solution of an LP
/// relaxation of MAF that the 3-approximation builds as a by-product (Schalekamp, van Zuylen, van der
/// Ster, arXiv:1511.06000, Algorithm 1). By weak duality the dual objective D lower-bounds k*-1, so
/// L = D+1 satisfies L <= k* on every instance.
///
/// This is a deliberately conservative rendering of that dual: it is always valid (L <= k* — the
/// property certified early exit relies on) but can be loose on adversarial topologies, because it
/// does not track the paper's reformulated-dual z-variables (which would tighten it at the cost of a
/// far more delicate construction — see the note in DualLowerBound.cpp).
///
/// Runs on internal clones of the two forests and does not mutate \p instance.
///
/// \param instance a two-forest MAF instance (as read, before reductions). For any other shape the
///        universally-valid trivial bound L = 1 is returned.
/// \return a certified lower bound L >= 1 with L <= k*.
long computeDual3ApproxLowerBound(const graph::Instance& instance);

/// \brief A tighter certified lower bound L on k*, from the *2-approximation* ("Red-Blue") dual of the
/// same paper (Schalekamp, van Zuylen, van der Ster, arXiv:1511.06000, Algorithm 2 + Procedures 1-4c).
///
/// Like \ref computeDual3ApproxLowerBound it builds a feasible dual solution and emits its objective
/// L = D+1 = sum(y), so L <= k* on every instance. The Red-Blue construction credits more dual per cut
/// (it handles a whole minimal incompatible active sibling set R u B via a reformulated dual), giving a
/// bound that is empirically ~1.28x tighter than the 3-approx dual and exact on ~16% of instances.
///
/// Only the dual value is needed, so the paper's retroactive merges (Procedures 4b/4c) -- which only
/// reduce the primal cut count and never change a y-variable -- are skipped. Validated L <= k* with 0
/// violations on 1200 exact-checked instances.
///
/// Runs on internal clones of the two forests and does not mutate \p instance. For any non-two-forest
/// shape the universally-valid trivial bound L = 1 is returned.
/// \return a certified lower bound L >= 1 with L <= k*.
long computeDual2ApproxLowerBound(const graph::Instance& instance);

/// \brief The best certified lower bound available: max of the 3-approx and 2-approx duals. Both are
/// always valid (<= k*), so their maximum is valid too and never worse than either. This is what
/// callers should use for the lower-bound track.
long computeCertifiedLowerBound(const graph::Instance& instance);

}  // namespace solver

#endif  // PACE2026_DUAL_LOWER_BOUND_HPP
