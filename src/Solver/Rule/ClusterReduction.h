//
// Created by Philip Kail on 3/3/26.
//

#ifndef PACE2026_CLUSTERREDUCTION_H
#define PACE2026_CLUSTERREDUCTION_H

#include "AbstractRule.hpp"

namespace solver
{


/// \brief Generates a list of cluster points across all trees and derives a partial instance with partial trees.
class ClusterReduction : public AbstractRule
{
    /// \brief This partialInstance holds partial Forests created through clustering.
    std::shared_ptr<graph::Instance> partialInstance;




  public:

    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    ClusterReduction(
        const std::shared_ptr<graph::Instance>& instance, const std::shared_ptr<Context>& context);



    /// \brief Decouples PartialForests of partialInstance from their respective Forest.
    /// \returns ?
    RuleReturnCode apply() override;


    /// \brief Reconnects PartialForests of partialInstance to their respective Forest.
    /// \returns ?
    void unapply() override;


    /// \brief Attempts to generate a list of viable cluster points for the instance.
    /// With that list the partialInstance will also be generated.
    /// \note This will employ the LCA and syncing of interior twins.
    /// \param instance on which the rule should be applied
    /// \param context contains additional information to the instance and the solver state
    /// \returns shared_pointer to ClusterReduction iff cluster points exist, null otherwise.
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context)




};


}
#endif  //PACE2026_CLUSTERREDUCTION_H
