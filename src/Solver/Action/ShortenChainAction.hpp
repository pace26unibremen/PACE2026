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

        void propagateBitmaskToRoot(const std::vector<uint64_t> bitmask, graph::Node* node) const;

    public:
        ShortenChainAction(unsigned int lower, unsigned int upper, const std::shared_ptr<graph::Forest>& forest);

        void doAction() override;

        void undoAction() override;

    };

}

#endif  //PACE2026_SHORTENCHAINACTION_HPP
