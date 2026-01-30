//
// Created by kaufm on 25.01.2026.
//

#ifndef PACE2026_DELETENODEACTION_H
#define PACE2026_DELETENODEACTION_H

#include "AbstractAction.hpp"
#include "../../Graph/Forest.hpp"

namespace solver
{

class DeleteNodeActionInChains : AbstractAction
{
    protected:
        std::shared_ptr<graph::Forest> forest;
        graph::Node node;
        graph::Node childNode;
        bool terminalOnRightSide;
        int toBeRemovedChildNodeIndex;

      public:;
        DeleteNodeActionInChains(graph::Node node, const std::shared_ptr<graph::Forest>& forest);
        void doAction();
        void undoAction();


};

}  //namespace solver

#endif  //PACE2026_DELETENODEACTION_H
