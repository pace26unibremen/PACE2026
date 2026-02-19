#ifndef PACE2026_RULE_RETURN_CODE_HPP
#define PACE2026_RULE_RETURN_CODE_HPP

namespace solver
{

/// \enum RuleReturnCode
/// \brief Enum for the return code of the \ref AbstractRule::apply method
enum class RuleReturnCode
{
    /// Imidate end the solver and return without solution.
    ImidateReturn,
    /// Continue calculation as normal.
    Continue,
    /// Continue calculation, rule suggests for further rules.
    ContinueWithRuleSuggestion,
    /// End the branch without retrieving a solution.
    CutBranch,
    /// End the branch and consider the instance as a solution.
    EndBranchWithSolutionCandidate,
};

} // namespace solver

#endif  //PACE2026_RULE_RETURN_CODE_HPP
