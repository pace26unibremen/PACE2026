//
// Created by Andre on 11.12.2025.
//

#include "ChainReductionRule.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <stack>
#include <string>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Forest>& T1,
    const std::shared_ptr<graph::Forest>& T2,
    const std::vector<std::list<int>>& chains
    )
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->T1 = T1;
    this->T2 = T2;
    this->chains = chains;
    std::stack<AbstractRule> changes = std::stack<AbstractRule>();
}

//Chain def:Let T be a rooted phylogenetic X-tree, and let C =
//(x1, x2, . . . , xn) be a sequence of elements in X with n ≥ 2.
//X:= Set of labels, bijectively used on leaves in the tree.

//We say that C is an n-chain (or short chain) of T
//if the parent of x1 coincides with the parent of x2
//or the parent of x2 is the parent of the parent of x1,
//and, for each i ∈ {3, 4, . . . , n},
//the parent of xi is the parent of the parent of xi−1.


void solver::ChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

}

void solver::ChainReductionRule::unapply()
{

}

std::shared_ptr<solver::AbstractRule>
solver::ChainReductionRule::isApplicable(const std::shared_ptr<graph::Forest>& T1,
    const std::shared_ptr<graph::Forest>& T2)
{
    //List of chains in the tree
    std::vector<std::list<int>> chainList;

    int size;

    //Fetch Nodes
    std::vector<graph::Node> treeOneNodes = T1->Nodes();
    std::vector<graph::Node> treeTwoNodes = T2->Nodes();

    //Fetch Leaves - Terminal Index to Label
    std::unordered_map<int,unsigned int> termIndexTreeOne = T1->Terminals();
    std::unordered_map<int,unsigned int> termIndexTreeTwo = T2->Terminals();

    if (termIndexTreeOne.size() < termIndexTreeTwo.size())
    {
        size = termIndexTreeTwo.size();
    }
    else
    {
        size = termIndexTreeOne.size();
    }

    //For every node that could be shared between both...
    for (int currentNodeIndex = 0; currentNodeIndex < size; currentNodeIndex++)
    {
        //x2 for current x1 in T1
        int siblingNodeT1 = treeOneNodes.at(currentNodeIndex).siblingIndex;

        //x3 for case 1
        int parentNodeT1 = treeOneNodes.at(currentNodeIndex).parentIndex;
        int parentNodeT2 = treeTwoNodes.at(currentNodeIndex).parentIndex;

        //x3 for case 2
        int x3T1 = treeOneNodes.at(parentNodeT1).parentIndex;
        int x3T2 = treeTwoNodes.at(parentNodeT2).parentIndex;
        //def x2 for case 2
        int case2siblingT1 = -1;
        int case2siblingT2 = -1;

        //List of chain elements
        std::list<int> commonChainNodes;

        //If the left child of x3 is x2, then set case2sibling to be right side, vice versa if not
        if (treeOneNodes.at(x3T1).leftChildIndex == parentNodeT1)
        {
            case2siblingT1 = treeOneNodes.at(x3T1).rightChildIndex;
        }
        else
        {
            case2siblingT1 = treeOneNodes.at(x3T1).leftChildIndex;
        }

        if (treeTwoNodes.at(x3T2).leftChildIndex == parentNodeT2)
        {
            case2siblingT2 = treeTwoNodes.at(x3T2).rightChildIndex;
        }
        else
        {
            case2siblingT2 = treeTwoNodes.at(x3T2).leftChildIndex;
        }


        // Case 1: If sibling of x1, x2, is equal across T1 and T2,
        // and their parent node match in both, then they form a 3-common
        if (siblingNodeT1 == treeTwoNodes.at(currentNodeIndex).siblingIndex &&
            treeOneNodes.at(siblingNodeT1).parentIndex == treeTwoNodes.at(siblingNodeT1).parentIndex &&
            parentNodeT1 == treeTwoNodes.at(currentNodeIndex).parentIndex)
        {
            //x3 von x1 und x2
            int parentNode = treeOneNodes.at(currentNodeIndex).parentIndex;


            //3-common-chain
            commonChainNodes = {currentNodeIndex, siblingNodeT1, parentNode};

            bool isCurrentParentEqual = true;

            //Go up the
            while (isCurrentParentEqual)
            {
                //Parents of current parent node in chain
                int currentParentT1 = treeOneNodes.at(parentNode).parentIndex;
                int currentParentT2 = treeTwoNodes.at(parentNode).parentIndex;

                if (currentParentT1 == currentParentT2)
                {
                    //Not a root check
                    if (currentParentT1!= -1 && currentParentT2 != -1)
                    {
                        commonChainNodes.push_back(currentParentT1);
                        parentNode = treeOneNodes.at(parentNode).parentIndex;
                    }
                    else
                    {
                        isCurrentParentEqual = false;
                    }
                }
                else
                {
                    isCurrentParentEqual = false;
                }

            }

            //Falls n>=4...
            //Chain reduction. For n ≥ 4, let C = (x1, x2, . . . , xn) be a
            //maximal n-chain that is common to T and T' . Then set
            //S = T |X \ {x4, x5, . . . , xn} and S' = T'|X \ {x4, x5, . . . , xn}.

            //Execute removal of chain from x4 onwards
            if (commonChainNodes.size() >= 4)
            {

            }
        }
        //Case 2: If instead: the parent of x2 is the parent of the parent of x1 in both trees,
        else if (x3T1 == x3T2 && case2siblingT1 == case2siblingT2)
        {
            commonChainNodes = {currentNodeIndex,case2siblingT1,x3T1};

            //Check
            bool anyChainableParents = true;

            //Find all parents in chain
            while (anyChainableParents)
            {
                int c2currentParentT1 = treeOneNodes.at(commonChainNodes.back()).parentIndex;
                int c2currentParentT2 = treeTwoNodes.at(commonChainNodes.back()).parentIndex;

                if ( (c2currentParentT1 != -1 && c2currentParentT2 != -1) && c2currentParentT1 == c2currentParentT2)
                {
                    commonChainNodes.push_back(c2currentParentT1);
                }
                else
                {
                    anyChainableParents = false;
                }
            }

            //Execute removal of chain from x4 onwards
            if (commonChainNodes.size() >= 4)
            {

            }
        }
    }
    // int maximumSharedNodes = sizeOfSharedNodes(treeOneNodes,treeTwoNodes);
    //Enumerate over all

}


