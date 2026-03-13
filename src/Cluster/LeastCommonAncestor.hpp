//
// Created by user on 3/7/26.
//

#ifndef PACE2026_LEASTCOMMONANCESTOR_HPP
#define PACE2026_LEASTCOMMONANCESTOR_HPP

#include "../Graph/Forest.hpp"

namespace cluster
{




/// \brief This class generates the table of least common ancestors for a given Forest.
/// It is necessary to generate cluster points.
class LeastCommonAncestor
{

  private:
      std::unordered_map<graph::Node*, int> nodesToPreorderNumber = std::unordered_map<graph::Node*, int>();
      std::shared_ptr<graph::Forest> forest;

      std::vector<int> preorderNumbers;

      std::vector<int> levelOfEulerTours;
      std::vector<int> firstOccurences;

      std::unordered_map<int,int> preorderToInternalPreorder = std::unordered_map<int,int>();
      std::vector<graph::Node *> preorderToNode;
      std::vector<std::vector<int> > RangeMinimumQuery;

      int generatePreorderNumbers(graph::Node* node, int preorderNumber);

      void eulerTour(graph::Node* node, int depth);

      void precomputeRangeMinimumQuery();
      int computeRangeMinimumQuery(unsigned long i, unsigned long j);



  public:

    /// \brief This is the constructor of the LCA Table.
    /// \param forest The forest of which we want to generate the LCA table for.
    explicit LeastCommonAncestor(std::shared_ptr<graph::Forest>& forest);


    /// \brief This function returns a node pointer to a least common ancestor of two node preorder numbers.
    /// Be very careful when employing this function, as the numbers are intended to be preorder numbers of another
    /// tree!
    /// \return Least Common Ancestor
    graph::Node* getLeastCommonAncestor(int preorderNumberA, int preorderNumberB);

    /// \brief This function returns a node pointer to a least common ancestor of two given nodes.
    /// The two given nodes MUST be from the forest which created the LCA object this operation is used upon.
    /// \return Least Common Ancestor
    graph::Node* getLeastCommonAncestor(graph::Node* firstNode, graph::Node* secondNode);

    /// \brief This function returns a map that links each Node* to their correspondingly generated preorderNumber.
    /// This is necessary for the generation of the twin-table because we want to fetch the preorderNumbers of the
    /// nodes of the "home" tree we want to link to each other tree.
    /// \note Internally, a value of this map is used and translated into elements of a "foreign" LCA instance.
    std::unordered_map<graph::Node*, int>* getNodesToPreorderNumber();


};

}


#endif  //PACE2026_LEASTCOMMONANCESTOR_HPP
