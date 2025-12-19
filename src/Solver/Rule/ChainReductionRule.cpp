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


//Chain reduction. For n ≥ 4, let C = (x1, x2, . . . , xn) be a
//maximal n-chain that is common to T and T' . Then set
//S = T |X \ {x4, x5, . . . , xn} and S' = T'|X \ {x4, x5, . . . , xn}.

//Chain def:Let T be a rooted phylogenetic X-tree, and let C =
//(x1, x2, . . . , xn) be a sequence of elements in X with n ≥ 2.
//We say that C is an n-chain (or short chain) of T if the
//parent of x1 coincides with the parent of x2 or the parent
//of x2 is the parent of the parent of x1, and, for each
//i ∈ {3, 4, . . . , n}, the parent of xi is the parent of the parent
//of xi−1.

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

    for (int currentNodeIndex = 0; currentNodeIndex < size; currentNodeIndex++)
    {
        //x2 für derzeitiges x1 in T1
        int siblingNode = treeOneNodes.at(currentNodeIndex).siblingIndex;


        //If x2 of x1 is equal across T1 and T2, and Parent of x2 equal in both trees, and parent of x1 equal in both
        //then 3-common-chain
        if (siblingNode == treeTwoNodes.at(currentNodeIndex).siblingIndex &&
            treeOneNodes.at(siblingNode).parentIndex == treeTwoNodes.at(siblingNode).parentIndex &&
            treeOneNodes.at(currentNodeIndex).parentIndex == treeTwoNodes.at(currentNodeIndex).parentIndex)
        {
            //x3
            int parentNode = treeOneNodes.at(currentNodeIndex).parentIndex;


            //3-common-chain
            std::list commonChainNodes = {currentNodeIndex, siblingNode, parentNode};

            bool isCurrentParentEqual = true;

            //Suche alle weitere gleichen Vaterknoten.
            while (isCurrentParentEqual)
            {
                //Parents of current parent node in chain
                int parentNodeT1 = treeOneNodes.at(parentNode).parentIndex;
                int parentNodeT2 = treeTwoNodes.at(parentNode).parentIndex;

                if (parentNodeT1 == parentNodeT2)
                {
                    //Not a root check
                    if (parentNodeT1 != -1 && parentNodeT2 != -1)
                    {
                        commonChainNodes.push_back(parentNodeT1);
                        parentNode = treeOneNodes.at(parentNode).parentIndex;
                    }
                    else
                    {
                        isCurrentParentEqual = false;
                    }
                }
                else
                {


                }
            }  //Das Terminiert irgendwann, keine sorge.

            //Falls n>=4...
            if (commonChainNodes.size() >= 4)
            {

            }


        }
    }

    // int maximumSharedNodes = sizeOfSharedNodes(treeOneNodes,treeTwoNodes);


    //Enumerate over all

}


