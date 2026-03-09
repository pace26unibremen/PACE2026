//
// Created by user on 3/7/26.
//

#include "LeastCommonAncestor.hpp"

#include <cmath>
#include "iostream"
namespace graph
{

LeastCommonAncestor::LeastCommonAncestor(std::shared_ptr<graph::Forest>& forestPointer)
{
    forest = forestPointer;
    graph::Node* rootNode = forest->Roots().front();

    initializePreorderNumbers(rootNode);
    generatePreorderNumbers(rootNode, 0);

    eulerTour(rootNode, 0);
    precomputeRangeMinimumQuery();
}

void LeastCommonAncestor::initializePreorderNumbers(graph::Node* node)
{

    nodesToPreorderNumber.emplace(node, uninitializedSlot);
    if (auto leftChild = node->leftChild)     initializePreorderNumbers(leftChild);
    if (auto rightChild = node->rightChild)         initializePreorderNumbers(rightChild);
}


int LeastCommonAncestor::generatePreorderNumbers(graph::Node* node, int preorderNumber)
{
    nodesToPreorderNumber[node] = preorderNumber;

    int current = preorderNumber;

    if (node->leftChild)
        current = generatePreorderNumbers(node->leftChild, current + 1);

    if (node->rightChild)
        current = generatePreorderNumbers(node->rightChild, current + 1);

    return current;
}

void LeastCommonAncestor::eulerTour(graph::Node* node, int depth)
{

    int preorderNumber = preorderToNode.size(), eulerNumber = preorderNumbers.size();

    preorderToNode.push_back(node);

    //if ( preorderToInternalPreorder.size() <= nodesToPreorderNumber[node])
    //    preorderToInternalPreorder.resize(nodesToPreorderNumber[node] + 1, uninitializedSlot);

    preorderToInternalPreorder[nodesToPreorderNumber.at(node)] = preorderNumber;

    firstOccurences.push_back(eulerNumber);
    levelOfEulerTours.push_back(depth);
    preorderNumbers.push_back(preorderNumber);




    if (auto leftChild = node->leftChild)
    {
        eulerTour(leftChild, depth + 1);
        levelOfEulerTours.push_back(depth);
        preorderNumbers.push_back(preorderNumber);
    }

    if (auto rightChild = node->rightChild)
    {
        eulerTour(rightChild, depth + 1);
        levelOfEulerTours.push_back(depth);
        preorderNumbers.push_back(preorderNumber);
    }
}

void LeastCommonAncestor::precomputeRangeMinimumQuery()
{
    RangeMinimumQuery.push_back(preorderNumbers);

    for (unsigned long length = 2; length <= preorderNumbers.size(); length *= 2)
    {
        std::vector<int> buffer = std::vector<int>();

        for (unsigned long start = 0; start < preorderNumbers.size() - length; ++start)
        {
            buffer.push_back(computeRangeMinimumQuery(start, start + length));
        }
        RangeMinimumQuery.push_back(buffer);
    }
}

int LeastCommonAncestor::computeRangeMinimumQuery(unsigned long i, unsigned long j)
{

    if (i == j)
        return preorderNumbers[i];

    int length = j - i - 1;

    if (length <= 0)
    {
        length = 0;
    }
    else
    {
        length = 31 - __builtin_clz(length);
    }


    int rmq1 = RangeMinimumQuery[length][i];

    int rmq2;

    if (length >= 0)
    {
        rmq2 = RangeMinimumQuery[length][j - (1 << (length))];
    }
    else
    {
        rmq2 = RangeMinimumQuery[length][j];
    }

    if (rmq1 < rmq2)
        return rmq2;

    return rmq2;
}


graph::Node* LeastCommonAncestor::getLeastCommonAncestor(int preorderNumberA, int preorderNumberB)
{
    int preorderA = preorderToInternalPreorder[preorderNumberA];
    int preorderB = preorderToInternalPreorder[preorderNumberB];



    int leastCommonAncestorIndex;

    if (preorderA <= preorderB)
    {
        leastCommonAncestorIndex = computeRangeMinimumQuery(firstOccurences[preorderA], firstOccurences[preorderB]);
    }
    else
    {
        leastCommonAncestorIndex = computeRangeMinimumQuery(firstOccurences[preorderB], firstOccurences[preorderA]);
    }

    return preorderToNode[leastCommonAncestorIndex];
}
std::unordered_map<graph::Node*, int>* LeastCommonAncestor::getNodesToPreorderNumber()
{
    return &nodesToPreorderNumber;
}

}