//
// Created by kaufm on 29.06.2026.
//

#include "ShortenChainAction.hpp"
#include <algorithm>
#include <ranges>
#include <vector>

namespace solver
{

     ShortenChainAction::ShortenChainAction(unsigned int lower, unsigned int upper,
         const std::shared_ptr<graph::Forest>& forest) :lower(lower), upper(upper), forest(forest)
    {}

    void ShortenChainAction::propagateBitmaskToRoot(graph::Node* node) const
    {
         graph::Node* root = nullptr;

         // Traverse up to the root, updating subtreeTerminals and sorting children
         while (node != nullptr)
         {
             root = node;
             // Update subtreeTerminals by removing child's terminals
             for (unsigned int i = 0; i < bitmaskReduceSubtreeTerminals.size(); i++)
             {
                 // Use OR to add terminals back
                 node->subtreeTerminals[i] ^= bitmaskReduceSubtreeTerminals[i];
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
     }

    unsigned int ShortenChainAction::recalculateChainSubtreeTerminals(graph::Node* upperNode, graph::Node* lowerNode)
     {
         if (!upperNode->leftChild || upperNode == lowerNode)
         {
             return upperNode->smallestTerminal();
         }

         unsigned int leftMinLabel = recalculateChainSubtreeTerminals(upperNode->leftChild, lowerNode);
         unsigned int rightMinLabel = recalculateChainSubtreeTerminals(upperNode->rightChild, lowerNode);

         if (leftMinLabel > rightMinLabel)
         {
             std::swap(upperNode->leftChild, upperNode->rightChild);
         }
            for(unsigned int i = 0; i < upperNode->subtreeTerminals.size(); i++)
         {
             upperNode->subtreeTerminals[i] =
                 upperNode->leftChild->subtreeTerminals[i] |
                 upperNode->rightChild->subtreeTerminals[i];
         }
         return std::min(leftMinLabel, rightMinLabel);

     }

    void ShortenChainAction::doAction()
    {
         auto upperNode = forest->LabelToTerminal().at(upper);
         auto lowerNode = forest->LabelToTerminal().at(lower);

         auto chainParent = upperNode->parent->parent;
         auto chainSibling = upperNode->parent->sibling;

         auto lowerParent = lowerNode->parent;

         upperParent = upperNode->parent;
         lowerUncle = lowerParent->sibling;

         lowerParent->sibling = chainSibling;
         lowerParent->parent = chainParent;

         chainSibling->sibling = lowerParent;

         if (chainParent->leftChild == chainSibling) chainParent->rightChild = lowerParent;
         else chainParent->leftChild = lowerParent;

         bitmaskReduceSubtreeTerminals = lowerParent->subtreeTerminals;

         for (unsigned int i = 0; i < bitmaskReduceSubtreeTerminals.size(); i++)
         {
             bitmaskReduceSubtreeTerminals[i] ^= upperNode->parent->subtreeTerminals[i];
         }
         propagateBitmaskToRoot(chainParent);

         auto root = forest->rootOf(chainParent);
         auto root_Iterator = std::ranges::find(forest->Roots(), root);
         forest->Roots().erase(root_Iterator, root_Iterator+1);

         root_Iterator = std::lower_bound(forest->Roots().begin(), forest->Roots().end(), root,
                             [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
         forest->Roots().insert(root_Iterator, root);

         std::unordered_set<unsigned int> chainLabels;
         for (size_t block = 0; block < bitmaskReduceSubtreeTerminals.size(); ++block) {
             uint64_t value = bitmaskReduceSubtreeTerminals[block];

             while (value != 0) {
                 uint64_t t = value & -value;
                 int bit_index = __builtin_ctzll(value);
                 unsigned int number = block * 64 + bit_index + 1;
                 chainLabels.insert(number);
                 value ^= t;
             }
         }

         for (auto label : chainLabels)
         {
             reductedLabelToNode.emplace(label, forest->LabelToTerminal().at(label));
         }

         //erase chain terminals out of maps in forest
         std::erase_if(forest->TerminalToLabel(), [chainLabels](const auto& entry){return chainLabels.contains(entry.second);});
         std::erase_if(forest->LabelToTerminal(), [chainLabels](const auto& entry){return chainLabels.contains(entry.first);});

    }

    void ShortenChainAction::undoAction()
    {
         //case 1
         //   chain parent
         //     ┌──-┴---──┐
         //lower parent
         // ┌─┴─┐
         //    lower
         auto lowerNode = forest->LabelToTerminal().at(lower);
         graph::Node* root;

         if (lowerNode->parent && lowerNode->parent->parent)
         {
             auto lowerParent = lowerNode->parent;
             auto chainParent = lowerParent->parent;
             auto chainSibling = lowerParent->sibling;


             lowerUncle->sibling = lowerParent;
             lowerParent->sibling = lowerUncle;

             lowerParent->parent = lowerUncle->parent;

             if (lowerUncle->parent->leftChild == lowerUncle) lowerUncle->parent->rightChild = lowerParent;
             else lowerUncle->parent->leftChild = lowerParent;


             upperParent->parent = chainParent;
             upperParent->sibling = chainSibling;

             chainSibling->sibling = upperParent;
             if (chainParent->leftChild == chainSibling) chainParent->rightChild = upperParent;
             else chainParent->leftChild = upperParent;

             root = forest->rootOf(lowerNode);
             auto rootIterator = std::ranges::find(forest->Roots(), root);
             forest->Roots().erase(rootIterator,rootIterator+1);
         }
         //case 2:
         //
         //lower parent
         // ┌─┴─┐
         //    lower
         else if (lowerNode->parent)
         {
             auto& lowerParent = lowerNode->parent;

             lowerUncle->sibling = lowerParent;
             lowerParent->sibling = lowerUncle;

             lowerParent->parent = lowerUncle->parent;
             if (lowerUncle->parent->leftChild == lowerUncle) lowerUncle->parent->rightChild = lowerParent;
             else lowerUncle->parent->leftChild = lowerParent;

             upperParent->parent = nullptr;
             upperParent->sibling = nullptr;

             //First subtree changes, then root changes

             root = upperParent;
             auto rootIterator = std::ranges::find(forest->Roots(), lowerParent);
             forest->Roots().erase(rootIterator,rootIterator+1);
         }
         else // lower is single vertex
         {
             lowerNode->sibling = lowerUncle;
             lowerNode->parent = lowerUncle->parent;
             lowerUncle->sibling = lowerNode;
             if (lowerUncle->parent->leftChild == lowerUncle) lowerUncle->parent->rightChild = lowerNode;
             else lowerUncle->parent->leftChild = lowerNode;

             upperParent->parent = nullptr;
             upperParent->sibling = nullptr;

             root = upperParent;
             auto rootIterator = std::ranges::find(forest->Roots(), lowerNode);
             forest->Roots().erase(rootIterator,rootIterator+1);
         }

         recalculateChainSubtreeTerminals(upperParent,lowerUncle->sibling);
         propagateBitmaskToRoot(upperParent->parent);

         auto rootIterator = std::lower_bound(forest->Roots().begin(), forest->Roots().end(), root,
                             [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
         forest->Roots().insert(rootIterator, root);

         for (const auto& [label,node] : reductedLabelToNode)
         {
             forest->LabelToTerminal().insert({label,node});
             forest->TerminalToLabel().insert({node,label});
         }
    }


}  //namespace solver
