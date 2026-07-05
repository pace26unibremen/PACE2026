#ifndef PACE2026_LABEL_TO_TERMINAL_MAP_HPP
#define PACE2026_LABEL_TO_TERMINAL_MAP_HPP

#include "Node.hpp"

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace graph
{

/// \brief Dense map from a terminal label to its \ref Node.
///
/// Terminal labels are small positive integers (the leaf labels 1..n, plus a
/// handful of synthetic labels minted by cluster decoupling). Because the key
/// space is dense and small we back the map with a flat \c std::vector<Node*>
/// indexed directly by label — a plain array lookup instead of a hash — which
/// removes the dominant hot-path cost of the branching solver (profiling showed
/// the \c std::unordered_map<unsigned,Node*> lookups were the single hottest
/// symbol).
///
/// Index \c label holds the terminal for that label; a \c nullptr slot means the
/// label is not present. Index 0 is never a valid label (labels start at 1) and
/// is treated as absent. The public surface mirrors the subset of the
/// \c std::unordered_map interface the codebase actually used
/// (\c at / \c operator[] / \c contains / \c erase / \c emplace / \c size and
/// iteration over present entries), so call sites stay unchanged.
class LabelToTerminalMap
{
    /// \brief Backing storage; \c data_[label] is the terminal, \c nullptr if absent.
    std::vector<Node*> data_;

    /// \brief Grows the backing vector so that \p label is a valid index.
    /// Uses geometric growth to keep insertion amortised O(1) even when labels
    /// arrive out of order or above the current maximum (cluster decoupling).
    void ensure(unsigned int label)
    {
        if (label >= data_.size())
        {
            const std::size_t geometric = data_.size() + data_.size() / 2 + 1;
            data_.resize(std::max<std::size_t>(label + 1, geometric), nullptr);
        }
    }

  public:
    LabelToTerminalMap() = default;

    /// \brief Reserves capacity for labels up to \p numberOfLabels.
    void reserve(std::size_t numberOfLabels) { data_.reserve(numberOfLabels + 1); }

    /// \brief Whether \p label maps to a terminal.
    [[nodiscard]] bool contains(unsigned int label) const
    {
        return label < data_.size() && data_[label] != nullptr;
    }

    /// \brief Modifiable slot for \p label, creating it (as \c nullptr) if absent.
    /// Mirrors \c std::unordered_map::operator[].
    Node*& operator[](unsigned int label)
    {
        ensure(label);
        return data_[label];
    }

    /// \brief Terminal for \p label. \throws std::out_of_range if not present.
    [[nodiscard]] Node* at(unsigned int label) const
    {
        if (label >= data_.size() || data_[label] == nullptr)
        {
            throw std::out_of_range("LabelToTerminalMap::at : label not present");
        }
        return data_[label];
    }

    /// \brief Inserts \p node under \p label if the label is not already present.
    /// Mirrors \c std::unordered_map::emplace (no overwrite of an existing key).
    void emplace(unsigned int label, Node* node)
    {
        ensure(label);
        if (data_[label] == nullptr)
        {
            data_[label] = node;
        }
    }

    /// \brief Inserts a \c {label, node} pair, mirroring \c std::unordered_map::insert
    /// (no overwrite of an existing key).
    void insert(std::pair<unsigned int, Node*> entry) { emplace(entry.first, entry.second); }

    /// \brief Removes \p label if present.
    void erase(unsigned int label)
    {
        if (label < data_.size())
        {
            data_[label] = nullptr;
        }
    }

    /// \brief Removes every present entry for which \p predicate returns true.
    /// Mirrors \c std::erase_if; the predicate receives a \c {label, node} pair.
    template <typename Predicate>
    void erase_if(Predicate predicate)
    {
        for (unsigned int label = 0; label < data_.size(); ++label)
        {
            if (data_[label] != nullptr && predicate(std::pair<unsigned int, Node*>{label, data_[label]}))
            {
                data_[label] = nullptr;
            }
        }
    }

    /// \brief Number of present labels. O(maxLabel); used only on cold paths.
    [[nodiscard]] std::size_t size() const
    {
        std::size_t count = 0;
        for (Node* p : data_)
        {
            if (p != nullptr) ++count;
        }
        return count;
    }

    /// \brief Largest present label, or 0 if empty.
    [[nodiscard]] unsigned int maxLabel() const
    {
        for (std::size_t label = data_.size(); label-- > 0;)
        {
            if (data_[label] != nullptr) return static_cast<unsigned int>(label);
        }
        return 0;
    }

    /// \brief Forward iterator over present \c (label, node) pairs, ascending by
    /// label. Dereferences to a \c std::pair by value (a proxy), so bind results
    /// with \c const \c auto& or \c auto — not a non-const reference.
    class const_iterator
    {
        const std::vector<Node*>* data_ = nullptr;
        std::size_t index_ = 0;

        void skipEmpty()
        {
            while (index_ < data_->size() && (*data_)[index_] == nullptr) ++index_;
        }

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<unsigned int, Node*>;
        using difference_type = std::ptrdiff_t;
        using reference = std::pair<unsigned int, Node*>;
        using pointer = void;

        const_iterator() = default;
        const_iterator(const std::vector<Node*>* data, std::size_t index) : data_(data), index_(index)
        {
            skipEmpty();
        }

        reference operator*() const
        {
            return {static_cast<unsigned int>(index_), (*data_)[index_]};
        }

        const_iterator& operator++()
        {
            ++index_;
            skipEmpty();
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const { return index_ == other.index_; }
        bool operator!=(const const_iterator& other) const { return index_ != other.index_; }
    };

    [[nodiscard]] const_iterator begin() const { return {&data_, 0}; }
    [[nodiscard]] const_iterator end() const { return {&data_, data_.size()}; }
};

}  // namespace graph

#endif  // PACE2026_LABEL_TO_TERMINAL_MAP_HPP
