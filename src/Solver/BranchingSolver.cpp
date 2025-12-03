#include "BranchingSolver.hpp"

solver::BranchingSolver::BranchingSolver(graph::Instance& instance)
{
    this->instance = std::move(instance);
}

graph::Forest solver::BranchingSolver::solve()
{

    // rules
    // 1. if instance size == 1 -> solution
    // 2. if instance i == instance j -> reduce continue
    // 4. if subtree ti equal in instance -> reduce continue
    // 5. if instance i == vertex only -> solution
    // 6. if instance pair path branching
    // 7. Debug Rule

    // graph_changes = []

    // class rule
    //   instance
    //
    //   apply()
    //   unapply()
    //   solvedInstance()
    //   static isApplicable() shared_pointer on rule / null
    //
    // class branchingRule : rule
    //   branch : int
    //
    //   isFullyExplored()
    //

    // rule_ptr = PathBranchingRule::isApplicable()
    //                      branch 0
    // rule_ptr.apply()     branch 1
    //
    // rule_ptr.unapply()
    // rule_ptr.apply()     branch 2
    // rule_ptr.unapply()
    // rule_ptr.apply()     branch 3
    // rule_ptr.unapply()
    // rule.



    // while true
    // {
    //      for rule in rules
    //          if rule.isApplicable()
    //              rule.apply()
    //              graph_changes.put(rule)
    //              if rule.solvedInstance()
    //                  // save solution
    //                  if solution_size > instance[].roots().size()
    //                      solution_size = instance[].roots().size()
    //                      solution = instance[0].copy
    //
    //                  // rebuild
    //                  while(true)
    //                  {
    //                       if(graph_changes.empty) // improve with fst branch counter
    //                           return solution[0]
    //                       rule = graph_changes.pop()
    //                       rule.undo()
    //                       if(rule is branching rule)
    //                       {
    //                            if(rule.isFullyExplored())
    //                               continue
    //                            else
    //                               rule.do()
    //                               break
    //                       }
    //                  }
    //
    // }

}