//
// Created by user on 3/7/26.
//

#ifndef PACE2026_LEASTCOMMONANCESTOR_HPP
#define PACE2026_LEASTCOMMONANCESTOR_HPP

#include "Forest.hpp"


namespace graph
{



struct parallelNodeData {
    std::unordered_map<graph::Node*, size_t>* nodesToPreorderNumber;
    graph::Node* parallelNode;
};


class LeastCommonAncestor
{
    const int uninitializedSlot = -1;


  std::unordered_map<graph::Node*, int> nodesToPreorderNumber = std::unordered_map<graph::Node*, int>();
  std::shared_ptr<graph::Forest> forest;

  std::vector<int> preorderNumbers; // E

  std::vector<int> levelOfEulerTours;		// L
  std::vector<int> firstOccurences;		// H

  //std::vector<int> preorderToInternalPreorder = std::vector<int>();    // T
  std::unordered_map<int,int> preorderToInternalPreorder = std::unordered_map<int,int>();    // T

  std::vector<graph::Node *> preorderToNode;	// N
  std::vector<std::vector<int> > RangeMinimumQuery;	// precomputed RMQ values

private:


  void initializePreorderNumbers(graph::Node* node);

  int generatePreorderNumbers(graph::Node* node, int preorderNumber);

  void eulerTour(graph::Node* node, int depth);

  void precomputeRangeMinimumQuery();

  int computeRangeMinimumQuery(unsigned long i, unsigned long j);



  public:

    explicit LeastCommonAncestor(std::shared_ptr<graph::Forest>& forest);
    graph::Node* getLeastCommonAncestor(int preorderNumberA, int preorderNumberB);

    std::unordered_map<graph::Node*, int>* getNodesToPreorderNumber();


};

}


#endif  //PACE2026_LEASTCOMMONANCESTOR_HPP
