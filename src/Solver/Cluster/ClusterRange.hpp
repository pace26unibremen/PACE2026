#ifndef PACE2026_CLUSTER_RANGE_HPP
#define PACE2026_CLUSTER_RANGE_HPP

#include "../../Graph/Instance.hpp"
#include "../Context.hpp"

namespace solver
{

class ClusterSolver;

class ClusterRange
{
  private:
    ClusterSolver* solver;
    void cutClusterTerminals(unsigned int index);
    void collectCuttedClusterRoot(unsigned int index);

  public:
    explicit ClusterRange(ClusterSolver* solver);

    class ClusterRangeIterator
    {
        friend class ClusterRange;

      private:
        unsigned int currentIndex = 0;
        ClusterRange* clusterRange;

      public:
        // define tags for STL function
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<std::shared_ptr<graph::Instance>, std::shared_ptr<Context>>;

        ClusterRangeIterator(unsigned int currentIndex, ClusterRange* clusterRange);

        value_type operator*() const;

        ClusterRangeIterator& operator++();

        ClusterRangeIterator operator++(int);

        bool operator==(const ClusterRangeIterator& other) const;

        bool operator!=(const ClusterRangeIterator& other) const;
    };

    [[nodiscard]] ClusterRangeIterator begin();

    [[nodiscard]] ClusterRangeIterator end();
};

}  // namespace solver

#endif  //PACE2026_CLUSTER_RANGE_HPP
