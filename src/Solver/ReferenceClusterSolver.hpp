//
// Created by user on 3/7/26.
//

#include "../Graph/InteriorTwinRelation.hpp"
#include "../Graph/LabelTwinRelation.hpp"
#include "AbstractSolver.hpp"

#ifndef PACE2026_REFERENCECLUSTERSOLVER_HPP
#define PACE2026_REFERENCECLUSTERSOLVER_HPP


namespace solver  {

/// \brief This is a solver that applies cluster reduction at the beginning of the run time to then undo it at
/// the very end. It is a reference reimplementation of the semantic thread as it can be derived from
/// within rSPR by Chris Whidden et al.
class ReferenceClusterSolver : public solver::AbstractSolver
{


  public:


    void recurseClustering()
    {

    }


    explicit ReferenceClusterSolver(const std::shared_ptr<graph::Instance>& instance);

    ~ReferenceClusterSolver() override = default;


    std::shared_ptr<graph::Forest> solve() override;


    void getAllNodePointersOfAForest(graph::Node* node, std::set<graph::Node*>* set);


};




}

#endif  //PACE2026_REFERENCECLUSTERSOLVER_HPP
