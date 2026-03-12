#ifndef PACE2026_DELETENODEACTION_H
#define PACE2026_DELETENODEACTION_H

#include "AbstractAction.hpp"
#include "../../Graph/Forest.hpp"

namespace solver
{

class DeleteNodeActionInChains : AbstractAction
{
    protected:
        /// \brief The Forest, in which the node param redies
        std::shared_ptr<graph::Forest> forest;

        /// \brief The node, which should be removed alongside his terminal child
        graph::Node node;

        /// \brief Check storer, which illustrates on which side the terminal child that is to be removed sides
        bool terminalOnRightSide;

        /// \brief The terminal child node, which will be removed alongside the param node
        graph::Node* toBeRemovedChildNode;

      public:
        /// \brief Action that deletes a node out of the forest given according to the chain reduction rule
        /// \param node The node to be removed
        /// \param forest The forest in which the node is to be removed out of
        DeleteNodeActionInChains(graph::Node* node, const std::shared_ptr<graph::Forest>& forest);

        /// \brief Deletes the node alongside the terminal child node out of the forest
        void doAction();

        /// \brief Undoes the deletion of the parameter and the terminal child node out of the forest
        void undoAction();


};

}  //namespace solver

#endif  //PACE2026_DELETENODEACTION_H
