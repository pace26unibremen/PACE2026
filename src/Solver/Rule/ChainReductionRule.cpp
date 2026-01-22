//
// Created by Andre on 11.12.2025.
//

#include "ChainReductionRule.h"

#include <algorithm>
#include <stack>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Forest>& T1,
    const std::shared_ptr<graph::Forest>& T2,
    const std::vector<std::vector<std::vector<int>>>& chains
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

    for (int index = 3; index < chains.size(); index++)
    {

    }

}

void solver::ChainReductionRule::unapply()
{

}

std::shared_ptr<solver::AbstractRule>
solver::ChainReductionRule::isApplicable(const std::shared_ptr<graph::Forest>& T1,
    const std::shared_ptr<graph::Forest>& T2)
{
    //List of chains in the tree.
    std::vector<std::vector<std::vector<int>>> chainList;


    //Fetch Nodes
    std::vector<graph::Node> treeOneNodes = T1->Nodes();
    std::vector<graph::Node> treeTwoNodes = T2->Nodes();

    //Fetch Leaves - Terminal Index to Label
    std::unordered_map<int,unsigned int> termIndexTreeOne = T1->Terminals();
    std::unordered_map<int,unsigned int> termIndexTreeTwo = T2->Terminals();

    //For all Terminals...
    for (const auto& terminalT1 : termIndexTreeOne)
    {   //Determine structure in T1
        int parentIndexT1 = treeOneNodes.at(terminalT1.first).parentIndex;

        // //Case 1 structure check
        // bool case1IsCurrentlyOnLeftSideT1;
        // if (treeOneNodes.at(parentIndexT1).leftChildIndex == terminalT1.first)
        // {
        //     case1IsCurrentlyOnLeftSideT1 = true;
        // }
        // else
        // {
        //     case1IsCurrentlyOnLeftSideT1 = false;
        // }

        //Determine if sibling exists
        int siblingIndexT1 = treeOneNodes.at(terminalT1.first).siblingIndex;

        // -> x1, x2 and x3 for case 1 for chain def known for T1


        //Determine x3 for case 2 for chain def in T1
        int parentOfParentIndexT1 = treeOneNodes.at(parentIndexT1).parentIndex;
        int siblingOfParentIndexT1 = treeOneNodes.at(parentIndexT1).siblingIndex;

        //Case 2 structure check
        bool case2SiblingIsOnLeftSideT1;
        if (treeOneNodes.at(parentOfParentIndexT1).leftChildIndex == siblingOfParentIndexT1)
        {
            case2SiblingIsOnLeftSideT1 = true;
        }
        else
        {
            case2SiblingIsOnLeftSideT1= false;
        }


        for (const auto& terminalT2 : termIndexTreeTwo)
        {
            //Determine Structure
            int parentIndexT2 = treeTwoNodes.at(terminalT2.first).parentIndex;
            // bool isCurrentOnLeftSideT2;
            // if (treeTwoNodes.at(parentIndexT2).leftChildIndex == terminalT2.first)
            // {
            //     isCurrentOnLeftSideT2 = true;
            // }
            // else
            // {
            //     isCurrentOnLeftSideT2 = false;
            // }
            int siblingIndexT2 = treeTwoNodes.at(terminalT2.first).siblingIndex;

            // -> x1,x2,x3 for case 1 known for T2
            //Determine x3 for case 2 for chain def in T2
            int parentOfParentIndexT2 = treeTwoNodes.at(parentIndexT2).parentIndex;
            int siblingOfParentIndexT2 = treeTwoNodes.at(parentIndexT2).siblingIndex;

            //Case 2 structure check
            bool case2SiblingIsOnLeftSideT2;
            if (treeTwoNodes.at(parentOfParentIndexT2).leftChildIndex == siblingOfParentIndexT2)
            {
                case2SiblingIsOnLeftSideT2 = true;
            }
            else
            {
                case2SiblingIsOnLeftSideT2= false;
            }

            //Case 1: if the parent of x1 coincides with the parent of x2
                if (siblingIndexT2 != -1 && siblingIndexT1 != -1 &&
                    (treeOneNodes.at(siblingIndexT1).parentIndex == parentIndexT1)
                    && (treeTwoNodes.at(siblingIndexT2).parentIndex == parentIndexT2))
                {
                    //Chain for both trees, x1-x3
                    std::vector<std::vector<int>> chainT1T2 = {{terminalT1.first,terminalT2.first},
                        {siblingIndexT1,siblingIndexT2},{parentIndexT1,parentIndexT2}};


                    bool case1check = true;

                    int case1T1Parent = treeOneNodes.at(chainT1T2.back().front()).parentIndex;
                    int case1T2Parent = treeTwoNodes.at(chainT1T2.back().back()).parentIndex;

                    int case1T1Sibling;


                    int case1T2Sibling;


                    //T1 structure check
                    if (treeOneNodes.at(case1T1Parent).leftChildIndex == chainT1T2.back().front())
                    {
                        case1T1Sibling = treeOneNodes.at(case1T1Parent).rightChildIndex;
                    }
                    else
                    {
                        case1T1Sibling = treeOneNodes.at(case1T1Parent).leftChildIndex;
                    }
                    //T2 structure c heck
                    if (treeOneNodes.at(case1T2Parent).leftChildIndex == chainT1T2.back().back())
                    {
                        case1T2Sibling = treeOneNodes.at(case1T1Parent).rightChildIndex;
                    }
                    else
                    {
                        case1T2Sibling = treeOneNodes.at(case1T2Parent).leftChildIndex;
                    }

                    //Determine x4,... of chain
                    while (case1check)
                    {   //Siblings exist
                        if (case1T1Sibling != -1 && case1T2Sibling != -1 && case1T1Parent != -1 && case1T2Parent != -1)
                        {
                            chainT1T2.push_back({case1T1Parent,case1T2Parent});
                            case1T1Parent = treeOneNodes.at(chainT1T2.back().front()).parentIndex;
                            case1T2Parent = treeTwoNodes.at(chainT1T2.back().back()).parentIndex;
                            //T1 structure check
                            if (treeOneNodes.at(case1T1Parent).leftChildIndex == chainT1T2.back().front())
                            {
                                case1T1Sibling = treeOneNodes.at(case1T1Parent).rightChildIndex;
                            }
                            else
                            {
                                case1T1Sibling = treeOneNodes.at(case1T1Parent).leftChildIndex;
                            }
                            //T2 structure c heck
                            if (treeOneNodes.at(case1T2Parent).leftChildIndex == chainT1T2.back().back())
                            {
                                case1T2Sibling = treeOneNodes.at(case1T1Parent).rightChildIndex;
                            }
                            else
                            {
                                case1T2Sibling = treeOneNodes.at(case1T2Parent).leftChildIndex;
                            }
                        }
                        else case1check = false;
                    }

                    if (chainT1T2.size() >= 4)
                    {
                        chainList.push_back(chainT1T2);
                    }
                }
            //Case 2: or the parent of x2 is the parent of the parent of x1,
                else if (case2SiblingIsOnLeftSideT1 == case2SiblingIsOnLeftSideT2 )
                {

                }
        }
    }
    return std::dynamic_pointer_cast<AbstractRule>(
        std::make_shared<ChainReductionRule>(T1,T2, chainList));
    // //For every node that could be shared between both...
    // for (int currentNodeIndex = 0; currentNodeIndex < size; currentNodeIndex++)
    // {
    //     //x2 for current x1 in T1
    //     int siblingNodeT1 = treeOneNodes.at(currentNodeIndex).siblingIndex;
    //
    //     //x3 for case 1
    //     int parentNodeT1 = treeOneNodes.at(currentNodeIndex).parentIndex;
    //     int parentNodeT2 = treeTwoNodes.at(currentNodeIndex).parentIndex;
    //
    //     //x3 for case 2
    //     int x3T1 = treeOneNodes.at(parentNodeT1).parentIndex;
    //     int x3T2 = treeTwoNodes.at(parentNodeT2).parentIndex;
    //     //def x2 for case 2
    //     int case2siblingT1 = -1;
    //     int case2siblingT2 = -1;
    //
    //     //List of chain elements
    //     std::list<int> commonChainNodes;
    //
    //     //If the left child of x3 is x2, then set case2sibling to be right side, vice versa if not
    //     if (treeOneNodes.at(x3T1).leftChildIndex == parentNodeT1)
    //     {
    //         case2siblingT1 = treeOneNodes.at(x3T1).rightChildIndex;
    //     }
    //     else
    //     {
    //         case2siblingT1 = treeOneNodes.at(x3T1).leftChildIndex;
    //     }
    //
    //     if (treeTwoNodes.at(x3T2).leftChildIndex == parentNodeT2)
    //     {
    //         case2siblingT2 = treeTwoNodes.at(x3T2).rightChildIndex;
    //     }
    //     else
    //     {
    //         case2siblingT2 = treeTwoNodes.at(x3T2).leftChildIndex;
    //     }
    //
    //
    //     // Case 1: If sibling of x1, x2, is equal across T1 and T2,
    //     // and their parent node match in both, then they form a 3-common
    //     if (siblingNodeT1 == treeTwoNodes.at(currentNodeIndex).siblingIndex &&
    //         treeOneNodes.at(siblingNodeT1).parentIndex == treeTwoNodes.at(siblingNodeT1).parentIndex &&
    //         parentNodeT1 == treeTwoNodes.at(currentNodeIndex).parentIndex)
    //     {
    //         //x3 von x1 und x2
    //         int parentNode = treeOneNodes.at(currentNodeIndex).parentIndex;
    //
    //
    //         //3-common-chain
    //         commonChainNodes = {currentNodeIndex, siblingNodeT1, parentNode};
    //
    //         bool isCurrentParentEqual = true;
    //
    //         //Go up the
    //         while (isCurrentParentEqual)
    //         {
    //             //Parents of current parent node in chain
    //             int currentParentT1 = treeOneNodes.at(parentNode).parentIndex;
    //             int currentParentT2 = treeTwoNodes.at(parentNode).parentIndex;
    //
    //             if (currentParentT1 == currentParentT2)
    //             {
    //                 //Not a root check
    //                 if (currentParentT1!= -1 && currentParentT2 != -1)
    //                 {
    //                     commonChainNodes.push_back(currentParentT1);
    //                     parentNode = treeOneNodes.at(parentNode).parentIndex;
    //                 }
    //                 else
    //                 {
    //                     isCurrentParentEqual = false;
    //                 }
    //             }
    //             else
    //             {
    //                 isCurrentParentEqual = false;
    //             }
    //
    //         }
    //
    //         //Falls n>=4...
    //         //Chain reduction. For n ≥ 4, let C = (x1, x2, . . . , xn) be a
    //         //maximal n-chain that is common to T and T' . Then set
    //         //S = T |X \ {x4, x5, . . . , xn} and S' = T'|X \ {x4, x5, . . . , xn}.
    //
    //         //Execute removal of chain from x4 onwards
    //         if (commonChainNodes.size() >= 4)
    //         {
    //
    //         }
    //     }
    //     //Case 2: If instead: the parent of x2 is the parent of the parent of x1 in both trees,
    //     else if (x3T1 == x3T2 && case2siblingT1 == case2siblingT2)
    //     {
    //         commonChainNodes = {currentNodeIndex,case2siblingT1,x3T1};
    //
    //         //Check
    //         bool anyChainableParents = true;
    //
    //         //Find all parents in chain
    //         while (anyChainableParents)
    //         {
    //             int c2currentParentT1 = treeOneNodes.at(commonChainNodes.back()).parentIndex;
    //             int c2currentParentT2 = treeTwoNodes.at(commonChainNodes.back()).parentIndex;
    //
    //             if ( (c2currentParentT1 != -1 && c2currentParentT2 != -1) && c2currentParentT1 == c2currentParentT2)
    //             {
    //                 commonChainNodes.push_back(c2currentParentT1);
    //             }
    //             else
    //             {
    //                 anyChainableParents = false;
    //             }
    //         }
    //
    //         //Execute removal of chain from x4 onwards
    //         if (commonChainNodes.size() >= 4)
    //         {
    //
    //         }
    //     }
    // }
    // int maximumSharedNodes = sizeOfSharedNodes(treeOneNodes,treeTwoNodes);
    //Enumerate over all

}


