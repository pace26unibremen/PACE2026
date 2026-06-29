//
// Created by kaufm on 29.06.2026.
//

#include "ShortenChainAction.hpp"
#include <algorithm>

namespace solver
{

     ShortenChainAction::ShortenChainAction(unsigned int lower, unsigned int upper,
         const std::shared_ptr<graph::Forest>& forest) :lower(lower), upper(upper), forest(forest)
    {}

    void ShortenChainAction::propagateBitmaskToRoot(const std::vector<uint64_t> bitmask, graph::Node* node) const
    {
         graph::Node* root = nullptr;

         // Traverse up to the root, updating subtreeTerminals and sorting children
         while (node != nullptr)
         {
             root = node;
             // Update subtreeTerminals by removing child's terminals
             for (unsigned int i = 0; i < bitmask.size(); i++)
             {
                 // Use OR to add terminals back
                 node->subtreeTerminals[i] ^= bitmask[i];
             }

             // sort children
             graph::Node& l = *node->leftChild;
             graph::Node& r = *node->rightChild;

             if (r.hasSmallestTerminal(&l))
             {
                 node->leftChild = &r;
                 node->rightChild = &l;
             }

             node = node->parent;
         }

         auto root_Iterator = std::ranges::find(forest->Roots(), root);
         forest->Roots().erase(root_Iterator, root_Iterator+1);

         root_Iterator = std::lower_bound(forest->Roots().begin(), forest->Roots().end(), root,
                             [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
         forest->Roots().insert(root_Iterator, root);
     }

    void ShortenChainAction::doAction()
    {
         auto upperNode = forest->LabelToTerminal().at(upper);
         auto lowerNode = forest->LabelToTerminal().at(lower);

         auto chainParent = upperNode->parent->parent;
         auto chainSibling = upperNode->parent->sibling;

         auto lowerParent = lowerNode->parent;

         lowerParent->sibling = chainSibling;
         lowerParent->parent = chainParent;

         chainSibling->sibling = lowerParent;

         if (chainParent->leftChild == chainSibling) chainParent->rightChild = lowerParent;
         else chainParent->leftChild = lowerParent;

         auto bitmaskReduceSubtreeTerminals = lowerParent->subtreeTerminals;

         for (unsigned int i = 0; i < bitmaskReduceSubtreeTerminals.size(); i++)
         {
             bitmaskReduceSubtreeTerminals[i] ^= upperNode->parent->subtreeTerminals[i];
         }
         propagateBitmaskToRoot(bitmaskReduceSubtreeTerminals, chainParent);


    }

    void ShortenChainAction::undoAction()
    {

    }


}  //namespace solver
