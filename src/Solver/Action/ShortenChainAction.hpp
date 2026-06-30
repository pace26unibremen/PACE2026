//
// Created by kaufm on 29.06.2026.
//

#ifndef PACE2026_SHORTENCHAINACTION_HPP
#define PACE2026_SHORTENCHAINACTION_HPP
#include "AbstractAction.hpp"
#include "../../Graph/Forest.hpp"

namespace solver
{

    class ShortenChainAction : public AbstractAction
    {
    private:
        unsigned int lower;
        unsigned int upper;
        std::shared_ptr<graph::Forest> forest;

        graph::Node* upperParent;
        graph::Node* lowerUncle;

        std::unordered_map<unsigned int, graph::Node*> reductedLabelToNode;

        std::vector<uint64_t> bitmaskReduceSubtreeTerminals;

        void propagateBitmaskToRoot(graph::Node* node) const;

        unsigned int recalculateChainSubtreeTerminals(graph::Node* upperNode, graph::Node* lowerNode);

    public:
        ShortenChainAction(unsigned int lower, unsigned int upper, const std::shared_ptr<graph::Forest>& forest);

        void doAction() override;

        void undoAction() override;

    };

}

#endif  //PACE2026_SHORTENCHAINACTION_HPP
