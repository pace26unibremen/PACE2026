//
// Created by kaufm on 15.06.2026.
//

#ifndef PACE2026_ADDEDGEACTION_HPP
#define PACE2026_ADDEDGEACTION_HPP
#include "AbstractAction.hpp"
#include "../../Graph/Forest.hpp"
#include "../../Graph/Node.hpp"

namespace solver
{

class AddEdgeAction : AbstractAction
{
    protected:
        graph::Node* child;
        graph::Node* parent;
        std::shared_ptr<graph::Forest> forest;
        bool parentFreeSideIsLeft;

    public:
    AddEdgeAction(graph::Node* child, graph::Node* parent, const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_ADDEDGEACTION_HPP
