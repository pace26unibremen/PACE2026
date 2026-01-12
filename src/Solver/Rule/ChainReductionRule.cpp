//
// Created by kaufm on 11.12.2025.
//

#include "ChainReductionRule.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <string>

solver::ChainReductionRule::ChainReductionRule()
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.


}

//Wie will man es machen?
//von Anzahl der Nodes bis 4...
// Zuerster Check: Gibt es zwei Nodes, welche Eltern teilen
// Dann: Aus den verbleibenen Nodes: Für die verbleibenen Nodes: Existiert bei jeder Runde ein Knoten, welcher der Parent
// des Parents der beiden knoten ist?

// int sizeOfSharedNodes(std::vector<graph::Node> t1Node, std::vector<graph::Node> t2Node)
// {
//     int highestSize = 0;
//
//     for (unsigned int i = 0; i < t1Node.size();  i++)
//     {
//         for (unsigned int j = 0; j < t2Node.size(); j++)
//         {
//             // if () WARTE BIS ADDRESSIERUNG DER NODES VERBESSERT WIRD
//             // {
//             //     highestSize++;
//             // }
//         }
//     }
//
//     return highestSize;
// }
//Forest stores all vertex with a nummer label. TerminalIndexToLabel orders the index of the leafs/terminals to the
//label, LabelToTerminalIndex the opposite.






//SO: First step: Identify Chains of n>= 4 that are present in T and T'.
//Second Step: Apply the reduction to both Forests for each chain.

//First step: Go through all element pairs with size n >= 4 on both T and T'
// IFF a Chain that is common in T and T' is found, then apply the subtree/chain deletion by removing the subtree.
// (I think that atm removing the edge between the parent of the node of the to the removed subtree/chain is the way to
// do that)
// Do this for all possible pairs and either quit the func if both trees were accessible during execution of func, or
// return both now reduced trees.


//Func to return all combinations

//we need to do this, since if a node is unique to only one, then it can't be removed.

void ChainReductionExecution(std::shared_ptr<graph::Forest>& T1, std::shared_ptr<graph::Forest>& T2)
{

    int size;
    if (T1->Nodes().size() > T2->Nodes().size())
    {
        size = T1->Nodes().size();
    }
    else
    {
        size = T2->Nodes().size();
    }

    //Fetch Nodes
    std::vector<graph::Node> treeOneNodes = T1->Nodes();
    std::vector<graph::Node> treeTwoNodes = T2->Nodes();

    // //Fetch Terminals
    // std::unordered_map<int,unsigned int> termIndexTreeOne = T1->Terminals();
    // std::unordered_map<int,unsigned int> termIndexTreeTwo = T2->Terminals();

    //Chain def:Let T be a rooted phylogenetic X-tree, and let C =
    //(x1, x2, . . . , xn) be a sequence of elements in X with n ≥ 2.

    //We say that C is an n-chain (or short chain) of T if the
    //parent of x1 coincides with the parent of x2 or the parent
    //of x2 is the parent of the parent of x1, and, for each
    //i ∈ {3, 4, . . . , n}, the parent of xi is the parent of the parent
    //of xi−1.

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
        }
    }

    // int maximumSharedNodes = sizeOfSharedNodes(treeOneNodes,treeTwoNodes);


    //Enumerate over all

}


