#include "DualLowerBound.hpp"

#include "../Graph/Forest.hpp"
#include "../Graph/Node.hpp"

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------------------------
// The dual objective is tracked as a single running integer `dualSum` (= sum of the paper's dual
// variables y): every finalized leaf contributes +1, every internal-node decrement contributes -1,
// and each is applied at most once per event, so the running sum equals sum(y) exactly. Then
// D = dualSum - 1 and the emitted lower bound is L = D + 1 = max(1, dualSum).
//
// The construction is deliberately conservative rather than the paper's fully faithful dual: it omits
// the reformulated-dual z-variables. A faithful version would raise more y-credit per conflict and so
// give a tighter (larger) L, but keeping it valid (L <= k*) under that extra credit is delicate --
// an over-credit anywhere makes L > k* and would certify an invalid answer. This rendering only ever
// credits +1 per finalized leaf and debits at conflict LCAs / cut pendants, which stays valid.
//
// Performance: every node gets a fixed integer index up front, and the current tree topology is held in
// parent/left/right index arrays (updated in O(1) on each cut). Per iteration the active-leaf count of
// every node is recomputed in a single O(n) array sweep (no hashing in the hot loop), and cherry finding
// is an O(n) array scan. That keeps the whole pass O(n^2) -- the paper's bound -- with small constants.
// ---------------------------------------------------------------------------------------------

namespace
{
using graph::Node;
using Label = unsigned int;
using ActiveSet = std::unordered_set<Label>;

constexpr int NO_NODE = -1;

[[maybe_unused]] bool isLeaf(const Node* n)
{
    // A cut turns an internal node unary (one child nulled); a genuine leaf has no children at all.
    return n->leftChild == nullptr && n->rightChild == nullptr;
}

/// A mutable clone of one input forest, indexed for fast active-leaf counting.
///
/// Every node has a fixed index (its position in `nodes`, which is a pre-order listing, so a parent's
/// index is always smaller than any of its descendants'). The live tree shape is mirrored in the
/// `parentIdx`/`leftIdx`/`rightIdx` arrays; \ref cut keeps both the node pointers and those arrays in
/// sync, so the count sweep and cherry scan never touch a hash map.
struct ForestView
{
    graph::Forest forest;  // owns the cloned nodes
    std::vector<Node*> nodes;
    std::unordered_map<const Node*, int> indexOf;
    std::unordered_map<Label, Node*> nodeOf;  // label -> leaf node
    std::vector<int> labelOf;                 // node index -> label, or 0 for internal nodes
    std::vector<int> parentIdx, leftIdx, rightIdx;
    std::vector<int> counts;  // reused active-leaf-count buffer, refilled each sweep

    explicit ForestView(const graph::Forest& source) : forest(source.copy())
    {
        // Collect nodes in pre-order (root before its descendants) and assign each a fixed index.
        for (Node* root : forest.Roots())
        {
            std::vector<Node*> stack{root};
            while (not stack.empty())
            {
                Node* n = stack.back();
                stack.pop_back();
                indexOf[n] = static_cast<int>(nodes.size());
                nodes.push_back(n);
                if (n->leftChild)
                    stack.push_back(n->leftChild);
                if (n->rightChild)
                    stack.push_back(n->rightChild);
            }
        }

        const int size = static_cast<int>(nodes.size());
        labelOf.assign(size, 0);
        parentIdx.assign(size, NO_NODE);
        leftIdx.assign(size, NO_NODE);
        rightIdx.assign(size, NO_NODE);
        counts.assign(size, 0);
        for (const auto& [label, node] : forest.LabelToTerminal())
        {
            nodeOf[label] = node;
            labelOf[indexOf.at(node)] = static_cast<int>(label);
        }
        for (int i = 0; i < size; ++i)
        {
            Node* n = nodes[i];
            if (n->parent)
                parentIdx[i] = indexOf.at(n->parent);
            if (n->leftChild)
                leftIdx[i] = indexOf.at(n->leftChild);
            if (n->rightChild)
                rightIdx[i] = indexOf.at(n->rightChild);
        }
    }

    int idx(const Node* n) const { return indexOf.at(n); }

    Node* node(Label label) const { return nodeOf.at(label); }

    /// Detach the edge above \p n (\p n becomes a new root), updating both the node pointers and the
    /// parent/child index arrays. No-op if \p n is already a root.
    void cut(Node* n)
    {
        Node* p = n->parent;
        if (p == nullptr)
            return;
        const int a = indexOf.at(n);
        const int pi = indexOf.at(p);
        if (p->leftChild == n)
        {
            p->leftChild = nullptr;
            leftIdx[pi] = NO_NODE;
        }
        else if (p->rightChild == n)
        {
            p->rightChild = nullptr;
            rightIdx[pi] = NO_NODE;
        }
        n->parent = nullptr;
        parentIdx[a] = NO_NODE;
    }

    /// Refill \ref counts with the number of active leaves in each node's subtree, in one O(n) sweep.
    void refreshCounts(const ActiveSet& active)
    {
        const int size = static_cast<int>(nodes.size());
        for (int i = 0; i < size; ++i)
        {
            const int label = labelOf[i];
            counts[i] = (label != 0 && active.contains(static_cast<Label>(label))) ? 1 : 0;
        }
        // nodes is pre-order, so descending i visits every child before its parent.
        for (int i = size - 1; i >= 0; --i)
            if (parentIdx[i] != NO_NODE)
                counts[parentIdx[i]] += counts[i];
    }

    int countAt(int i) const { return i == NO_NODE ? 0 : counts[i]; }

    Node* rootOf(Node* n) const
    {
        while (n->parent != nullptr)
            n = n->parent;
        return n;
    }
};

/// LCA of \p a and \p b within the current forest, or nullptr if they are in different components.
Node* lca(Node* a, Node* b)
{
    std::unordered_set<const Node*> anc;
    for (Node* x = a; x != nullptr; x = x->parent)
        anc.insert(x);
    for (Node* x = b; x != nullptr; x = x->parent)
        if (anc.contains(x))
            return x;
    return nullptr;
}

/// The two active leaves in the subtree of node index \p start (which holds exactly two). Descends only
/// into children that still carry an active leaf, so this is O(depth), not O(subtree).
std::pair<Label, Label> twoActiveLeavesUnder(const ForestView& view, int start)
{
    std::vector<Label> found;
    std::vector<int> stack{start};
    while (not stack.empty() && found.size() < 2)
    {
        const int i = stack.back();
        stack.pop_back();
        if (view.labelOf[i] != 0)
        {
            found.push_back(static_cast<Label>(view.labelOf[i]));
        }
        else
        {
            if (view.leftIdx[i] != NO_NODE && view.counts[view.leftIdx[i]] > 0)
                stack.push_back(view.leftIdx[i]);
            if (view.rightIdx[i] != NO_NODE && view.counts[view.rightIdx[i]] > 0)
                stack.push_back(view.rightIdx[i]);
        }
    }
    return {found[0], found[1]};
}

/// The lowest internal node whose subtree holds exactly two active leaves (an active sibling pair),
/// read from the precomputed counts. Returns the two leaf labels, or nullopt if none exists. Scans in
/// pre-order and requires both children to hold < 2, so it returns the pre-order-first lowest such node
/// (identical selection to the reference construction).
std::optional<std::pair<Label, Label>> findActiveCherry(const ForestView& view)
{
    const int size = static_cast<int>(view.nodes.size());
    for (int i = 0; i < size; ++i)
    {
        if (view.labelOf[i] != 0 || view.counts[i] != 2)
            continue;
        if (view.countAt(view.leftIdx[i]) >= 2 || view.countAt(view.rightIdx[i]) >= 2)
            continue;
        return twoActiveLeavesUnder(view, i);
    }
    return std::nullopt;
}

bool isActiveSiblingPair(const ForestView& view, Node* aNode, Node* bNode)
{
    if (view.rootOf(aNode) != view.rootOf(bNode))
        return false;
    Node* m = lca(aNode, bNode);
    return m != nullptr && view.counts[view.idx(m)] == 2;
}

/// If u,v sit in the same component separated by an active pendant, return (W, p2W): the first off-path
/// subtree carrying an active leaf on the u-v path and its attachment (path) node. This is the paper's
/// p2(W) / Whidden's b-node, walking up from u and from v toward (but not including) their LCA. Returns
/// nullopt when u,v are already adjacent (no active pendant between them).
std::optional<std::pair<Node*, Node*>> findBlockingW(const ForestView& view, Node* uNode, Node* vNode)
{
    Node* m = lca(uNode, vNode);
    if (m == nullptr)
        return std::nullopt;
    for (Node* leaf : {uNode, vNode})
    {
        Node* child = leaf;
        Node* node = leaf->parent;
        while (node != nullptr && node != m)
        {
            for (Node* ch : {node->leftChild, node->rightChild})
            {
                if (ch == nullptr || ch == child)
                    continue;
                if (view.counts[view.idx(ch)] > 0)
                    return std::make_pair(ch, node);
            }
            child = node;
            node = node->parent;
        }
    }
    return std::nullopt;
}
}  // namespace

long solver::computeDual3ApproxLowerBound(const graph::Instance& instance)
{
    // The dual construction of Algorithm 1 is defined for two rooted binary trees. For any other shape
    // fall back to the universally-valid trivial bound: every agreement forest has >= 1 component, so
    // L = 1 <= k*.
    if (instance.size() != 2)
        return 1;

    ForestView t1(*instance.at(0));
    ForestView t2(*instance.at(1));

    ActiveSet active;
    active.reserve(t1.nodeOf.size());
    for (const auto& [label, node] : t1.nodeOf)
        active.insert(label);

    long dualSum = 0;

    while (active.size() >= 2)
    {
        t1.refreshCounts(active);
        auto cherry = findActiveCherry(t1);
        if (not cherry.has_value())
            break;
        const Label u = cherry->first;
        const Label v = cherry->second;

        t2.refreshCounts(active);
        Node* uT2 = t2.node(u);
        Node* vT2 = t2.node(v);
        const int cu = t2.counts[t2.idx(t2.rootOf(uT2))];
        const int cv = t2.counts[t2.idx(t2.rootOf(vT2))];

        // line 5: u or v is alone in its T2 component -> finalize it as a singleton.
        if (cu == 1 || cv == 1)
        {
            const Label w = (cu == 1) ? u : v;
            t1.cut(t1.node(w));
            t2.cut(t2.node(w));
            active.erase(w);
            dualSum += 1;  // y_w <- 1
            continue;
        }

        // line 8: u,v are also an active sibling pair in T2 (a common cherry) -> merge, no dual change.
        if (isActiveSiblingPair(t2, uT2, vT2))
        {
            active.erase(u);  // u becomes inactive, represented by v
            continue;
        }

        // lines 10-13: conflict. Optionally cut a blocking active pendant W in T2 first.
        if (t2.rootOf(uT2) == t2.rootOf(vT2))
        {
            if (auto blocking = findBlockingW(t2, uT2, vT2))
            {
                t2.cut(blocking->first);
                dualSum -= 1;  // y_{p2(W)} <- y_{p2(W)} - 1
            }
        }

        // line 13: cut u and v off in both forests, finalize both, decrement y at lca1(u,v).
        Node* l1 = lca(t1.node(u), t1.node(v));
        t1.cut(t1.node(u));
        t1.cut(t1.node(v));
        t2.cut(uT2);
        t2.cut(vT2);
        active.erase(u);
        active.erase(v);
        dualSum += 2;  // y_u <- 1, y_v <- 1
        if (l1 != nullptr)
            dualSum -= 1;  // y_{lca1(u,v)} <- y_{lca1(u,v)} - 1
    }

    // line 18: finalize the last remaining active leaf.
    if (active.size() == 1)
        dualSum += 1;

    // D = dualSum - 1, L = D + 1 = dualSum, clamped to the trivial floor of 1 (>= 1 component).
    return dualSum > 1 ? dualSum : 1;
}

// =================================================================================================
// Red-Blue 2-approximation dual (Algorithm 2 + Procedures 1-4c). See DualLowerBound.hpp and, for the
// per-case boxed y-updates, proto/ALG2_SPEC.md in the lp-duality worktree. We track only dualSum = sum(y);
// the bound is L = dualSum. Retroactive merges (Proc 4b/4c) never touch y, so they are skipped.
//
// Two clones of each input forest are kept: a WORKING copy (mutated by cuts) for structural predicates
// (same tree / active siblings / alone-in-tree) and an ORIGINAL copy (never cut) for the topology
// relation "uv|w" and triplet consistency. Predicates walk subtrees directly against the active set
// rather than the counts array, so no count bookkeeping can go stale across the many mutation kinds.
// =================================================================================================
namespace
{
int labelOfNode(const ForestView& v, const Node* n) { return v.labelOf[v.indexOf.at(n)]; }

/// Active leaf labels in the subtree rooted at \p n of working/original view \p v.
std::vector<Label> activeLabelsUnder(const ForestView& v, Node* n, const ActiveSet& active)
{
    std::vector<Label> out;
    std::vector<Node*> stack{n};
    while (not stack.empty())
    {
        Node* x = stack.back();
        stack.pop_back();
        const int lbl = labelOfNode(v, x);
        if (lbl != 0)
        {
            if (active.contains(static_cast<Label>(lbl)))
                out.push_back(static_cast<Label>(lbl));
        }
        else
        {
            if (x->leftChild)
                stack.push_back(x->leftChild);
            if (x->rightChild)
                stack.push_back(x->rightChild);
        }
    }
    return out;
}

bool subtreeHasActive(const ForestView& v, Node* n, const ActiveSet& active)
{
    std::vector<Node*> stack{n};
    while (not stack.empty())
    {
        Node* x = stack.back();
        stack.pop_back();
        const int lbl = labelOfNode(v, x);
        if (lbl != 0)
        {
            if (active.contains(static_cast<Label>(lbl)))
                return true;
        }
        else
        {
            if (x->leftChild)
                stack.push_back(x->leftChild);
            if (x->rightChild)
                stack.push_back(x->rightChild);
        }
    }
    return false;
}

int activeCountUnder(const ForestView& v, Node* n, const ActiveSet& active)
{
    return static_cast<int>(activeLabelsUnder(v, n, active).size());
}

/// Depth (distance from root) of \p n via parent pointers.
int depthOf(Node* n)
{
    int d = 0;
    for (Node* x = n->parent; x != nullptr; x = x->parent)
        ++d;
    return d;
}

/// The Red-Blue dual construction as a self-contained pass over four ForestViews.
struct RedBlue2
{
    ForestView &t1, &t2;    // working (mutated)
    ForestView &t1o, &t2o;  // original (topology)
    ActiveSet& active;
    long dualSum = 0;
    bool sawStar = false;  // FinalCut or Merge-After-Cut seen in the current ResolveSet

    RedBlue2(ForestView& w1, ForestView& w2, ForestView& o1, ForestView& o2, ActiveSet& a)
        : t1(w1), t2(w2), t1o(o1), t2o(o2), active(a)
    {
    }

    // ---- topology on ORIGINAL trees ----
    /// The "outlier" of triplet {a,b,c} in original view \p v: the leaf split off from the closer pair
    /// (xy|z => outlier z). Returns 0 if degenerate. Uses the deepest of the three pairwise LCAs.
    Label outlier(ForestView& v, Label a, Label b, Label c)
    {
        Node* na = v.node(a);
        Node* nb = v.node(b);
        Node* nc = v.node(c);
        Node* lab = lca(na, nb);
        Node* lac = lca(na, nc);
        Node* lbc = lca(nb, nc);
        const int dab = lab ? depthOf(lab) : -1;
        const int dac = lac ? depthOf(lac) : -1;
        const int dbc = lbc ? depthOf(lbc) : -1;
        if (dab >= dac && dab >= dbc)
            return (dab > dac || dab > dbc) ? c : 0;  // a,b closest -> outlier c
        if (dac >= dab && dac >= dbc)
            return (dac > dab || dac > dbc) ? b : 0;
        return (dbc > dab || dbc > dac) ? a : 0;
    }

    bool consistent(Label a, Label b, Label c) { return outlier(t1o, a, b, c) == outlier(t2o, a, b, c); }

    /// uv|w in ORIGINAL tree \p v: u,v are the closer pair (outlier is w).
    bool restrictsO(ForestView& v, Label u, Label vv, Label w) { return outlier(v, u, vv, w) == w; }

    // ---- structural predicates on WORKING forests ----
    bool sameTree(ForestView& v, Label a, Label b) { return v.rootOf(v.node(a)) == v.rootOf(v.node(b)); }

    bool aloneInTree(ForestView& v, Label u)
    {
        return activeCountUnder(v, v.rootOf(v.node(u)), active) == 1;
    }

    bool activeSiblings(ForestView& v, Label a, Label b)
    {
        if (not sameTree(v, a, b))
            return false;
        Node* m = lca(v.node(a), v.node(b));
        return m != nullptr && activeCountUnder(v, m, active) == 2;
    }

    bool compatibleSet(const std::vector<Label>& s)
    {
        for (std::size_t i = 0; i < s.size(); ++i)
            for (std::size_t j = i + 1; j < s.size(); ++j)
                for (std::size_t k = j + 1; k < s.size(); ++k)
                    if (not consistent(s[i], s[j], s[k]))
                        return false;
        return true;
    }

    // ---- cut primitives on WORKING forests ----
    /// Cut the edge below p(u) toward u (detaches the subtree whose only active leaf is u). No-op if u is
    /// the only active leaf in its tree.
    void cutOffLeaf(ForestView& v, Label u)
    {
        Node* child = v.node(u);
        while (child->parent != nullptr)
        {
            Node* parent = child->parent;
            for (Node* sib : {parent->leftChild, parent->rightChild})
            {
                if (sib == nullptr || sib == child)
                    continue;
                if (subtreeHasActive(v, sib, active))
                {
                    v.cut(child);
                    return;
                }
            }
            child = parent;
        }
    }

    /// ResolvePair line 13: cut the active pendant hanging below p(u) that is NOT toward u.
    void cutPendant(ForestView& v, Label u)
    {
        Node* child = v.node(u);
        while (child->parent != nullptr)
        {
            Node* parent = child->parent;
            for (Node* sib : {parent->leftChild, parent->rightChild})
            {
                if (sib == nullptr || sib == child)
                    continue;
                if (subtreeHasActive(v, sib, active))
                {
                    v.cut(sib);
                    return;
                }
            }
            child = parent;
        }
    }

    /// True if p(u) is strictly below lca(u,v) in \p v (an active pendant strictly between u and lca).
    bool pBelowLca(ForestView& v, Label u, Label vv)
    {
        Node* m = lca(v.node(u), v.node(vv));
        Node* child = v.node(u);
        while (child->parent != nullptr && child->parent != m)
        {
            Node* parent = child->parent;
            for (Node* sib : {parent->leftChild, parent->rightChild})
            {
                if (sib == nullptr || sib == child)
                    continue;
                if (subtreeHasActive(v, sib, active))
                    return true;
            }
            child = parent;
        }
        return false;
    }

    /// Paper line 2 relabel test: does the ORIGINAL lca2(u,v) node currently sit in u's working T2 tree?
    /// The working (t2) and original (t2o) views are independent clones of the same forest built by the
    /// identical pre-order, so node index i denotes the same original node in both.
    bool origLca2InTreeOf(Label u, Label v)
    {
        Node* lo = lca(t2o.node(u), t2o.node(v));
        if (lo == nullptr)
            return false;
        Node* loWork = t2.nodes[t2o.indexOf.at(lo)];
        return t2.rootOf(loWork) == t2.rootOf(t2.node(u));
    }

    /// Does u's working T2 tree hold an active leaf other than u that is NOT in U? (FinalCut condition.)
    bool uTreeHasActiveOutsideU(Label u, const std::unordered_set<Label>& U)
    {
        for (Label w : activeLabelsUnder(t2, t2.rootOf(t2.node(u)), active))
            if (w != u && not U.contains(w))
                return true;
        return false;
    }

    // ---- ResolvePair (Procedure 1) ----
    void resolvePair(Label u, Label vv, const std::unordered_set<Label>& U)
    {
        if (not sameTree(t2, u, vv))
        {
            // line 2: relabel so lca2(u,v) is NOT in the tree containing u.
            if (origLca2InTreeOf(u, vv))
                std::swap(u, vv);
            if (uTreeHasActiveOutsideU(u, U))
            {
                cutOffLeaf(t2, u);
                cutOffLeaf(t1, u);
                active.erase(u);
                dualSum += 1;  // FinalCut, y_u <- 1
                sawStar = true;
            }
            else
            {
                cutOffLeaf(t1, u);  // line 6: cut in T1' only
                active.erase(u);
                dualSum += 1;  // y_u <- 1
            }
            return;
        }
        if (activeSiblings(t2, u, vv))
        {
            active.erase(u);  // merge, no dual change
            return;
        }
        if (not pBelowLca(t2, u, vv))
            std::swap(u, vv);
        cutPendant(t2, u);
        dualSum -= 1;  // y_{p2(u)} <- y_{p2(u)} - 1
        if (activeSiblings(t2, u, vv))
        {
            active.erase(u);  // Merge-After-Cut
            dualSum += 1;     // y_u <- 1
            sawStar = true;
        }
        else
        {
            cutOffLeaf(t1, u);
            cutOffLeaf(t2, u);
            active.erase(u);
            dualSum += 1;  // y_u <- 1
        }
    }

    std::vector<Label> activeIn(const std::vector<Label>& s)
    {
        std::vector<Label> out;
        for (Label x : s)
            if (active.contains(x))
                out.push_back(x);
        return out;
    }

    /// An active sibling pair in T1' with both leaves in \p s, or {0,0} if none.
    std::pair<Label, Label> activeSiblingPairIn(const std::vector<Label>& s)
    {
        std::vector<Label> act = activeIn(s);
        for (std::size_t i = 0; i < act.size(); ++i)
            for (std::size_t j = i + 1; j < act.size(); ++j)
                if (activeSiblings(t1, act[i], act[j]))
                    return {act[i], act[j]};
        return {0, 0};
    }

    // ---- ResolveSet (Procedure 2) ----
    bool resolveSet(const std::vector<Label>& R)  // returns true = Success
    {
        const std::unordered_set<Label> Uset(R.begin(), R.end());
        sawStar = false;
        dualSum -= 1;  // line 21: y_{lca1(R)} <- y_{lca1(R)} - 1
        while (static_cast<int>(activeIn(R).size()) >= 3)
        {
            auto [a, b] = activeSiblingPairIn(R);
            if (a == 0)
                break;
            resolvePair(a, b, Uset);
        }
        std::vector<Label> rem = activeIn(R);
        if (rem.size() < 2)
            return true;  // collapsed early (e.g. FinalCuts): progress made
        Label uh = rem[0], vh = rem[1];
        if (activeSiblings(t2, uh, vh))
        {
            active.erase(uh);
            dualSum += 1;  // merge, y_uh <- 1
            return true;
        }
        bool condDiff = not sameTree(t2, uh, vh);
        bool hasW = false;
        if (not condDiff)
        {
            for (Label w : activeLabelsUnder(t2, t2.rootOf(t2.node(uh)), active))
            {
                if (w == uh || w == vh)
                    continue;
                if (restrictsO(t2o, uh, vh, w))
                {
                    hasW = true;
                    break;
                }
            }
        }
        if (condDiff || not hasW)
        {
            if (not aloneInTree(t2, uh))
                cutOffLeaf(t2, uh);
            cutOffLeaf(t1, uh);
            active.erase(uh);
            dualSum += 1;  // y_uh <- 1
            if (not aloneInTree(t2, vh))
                cutOffLeaf(t2, vh);
            cutOffLeaf(t1, vh);
            active.erase(vh);
            dualSum += 1;  // y_vh <- 1
            return true;
        }
        if (sawStar)
        {
            resolvePair(uh, vh, Uset);
            std::vector<Label> last = activeIn(R);
            if (not last.empty())
            {
                Label lv = last[0];
                if (not aloneInTree(t2, lv))
                    cutOffLeaf(t2, lv);
                cutOffLeaf(t1, lv);
                active.erase(lv);
                dualSum += 1;  // y_vh <- 1
            }
            return true;
        }
        return false;  // Fail
    }

    // ---- minimal incompatible active sibling set ----
    Node* lcaSet(ForestView& v, const std::vector<Label>& labels)
    {
        if (labels.empty())
            return nullptr;
        Node* cur = v.node(labels[0]);
        for (std::size_t i = 1; i < labels.size() && cur != nullptr; ++i)
            cur = lca(cur, v.node(labels[i]));
        return cur;
    }

    /// Find (R, B) with R u B a minimal incompatible active sibling set; R has >=2 leaves. Returns false
    /// if none (all remaining active leaves are mutually compatible).
    bool findMinIncompat(std::vector<Label>& R, std::vector<Label>& B)
    {
        std::vector<Label> act(active.begin(), active.end());
        if (act.size() < 2)
            return false;
        Node* node = lcaSet(t1, act);
        while (node != nullptr)
        {
            std::vector<std::vector<Label>> childsets;
            for (Node* ch : {node->leftChild, node->rightChild})
                if (ch != nullptr)
                {
                    auto s = activeLabelsUnder(t1, ch, active);
                    if (not s.empty())
                        childsets.push_back(std::move(s));
                }
            if (childsets.size() < 2)
            {
                Node* nxt = nullptr;
                for (Node* ch : {node->leftChild, node->rightChild})
                    if (ch != nullptr)
                    {
                        auto s = activeLabelsUnder(t1, ch, active);
                        if (not s.empty())
                        {
                            nxt = lcaSet(t1, s);
                            break;
                        }
                    }
                if (nxt == nullptr || nxt == node)
                    return false;
                node = nxt;
                continue;
            }
            std::vector<Label>& A = childsets[0];
            std::vector<Label>& C = childsets[1];
            if (compatibleSet(A) && compatibleSet(C))
            {
                orient(A, C, R, B);
                return true;
            }
            node = lcaSet(t1, compatibleSet(A) ? C : A);
        }
        return false;
    }

    /// Orient so R has >=2 leaves (ResolveSet distills it to the pair r1,r2), preferring the
    /// lca2-spanning side (which is never a singleton).
    void orient(std::vector<Label>& A, std::vector<Label>& C, std::vector<Label>& R, std::vector<Label>& B)
    {
        std::vector<Label> uni;
        uni.insert(uni.end(), A.begin(), A.end());
        uni.insert(uni.end(), C.begin(), C.end());
        Node* mu = lcaSet(t2, uni);
        auto pick = [&](std::vector<Label>& X, std::vector<Label>& Y) {
            R = X;
            B = Y;
        };
        const bool aBig = A.size() >= 2, cBig = C.size() >= 2;
        if (mu != nullptr)
        {
            if (aBig && lcaSet(t2, A) == mu)
                return pick(A, C);
            if (cBig && lcaSet(t2, C) == mu)
                return pick(C, A);
        }
        if (aBig)
            return pick(A, C);
        if (cBig)
            return pick(C, A);
        pick(A.size() >= C.size() ? A : C, A.size() >= C.size() ? C : A);
    }

    // ---- final triplet procedures ----
    void proc4a(Label r1, Label r2, Label b)
    {
        if (not restrictsO(t2o, b, r1, r2))
            std::swap(r1, r2);  // ensure b r1 | r2 in original T2
        Node* m = lca(t2.node(r1), t2.node(b));
        if (m != nullptr && m->parent != nullptr)
            t2.cut(m);
        dualSum -= 1;  // y_{lca2(r1,b)} <- ... - 1
        cutOffLeaf(t2, r2);
        cutOffLeaf(t1, r2);
        active.erase(r2);
        dualSum += 1;  // y_r2 <- 1
        if (activeSiblings(t2, r1, b))
        {
            active.erase(b);
            dualSum += 1;  // merge b into r1, y_b <- 1
        }
        else
        {
            cutOffLeaf(t2, r1);
            cutOffLeaf(t1, r1);
            active.erase(r1);
            cutOffLeaf(t2, b);
            cutOffLeaf(t1, b);
            active.erase(b);
            dualSum += 2;  // y_r1 <- 1, y_b <- 1
        }
    }

    void proc4b(Label r1, Label r2, Label b)
    {
        cutOffLeaf(t2, b);
        cutOffLeaf(t1, b);
        active.erase(b);
        dualSum += 1;  // y_b <- 1
        for (Label r : {r1, r2})
        {
            if (not aloneInTree(t2, r))
                cutOffLeaf(t2, r);
            cutOffLeaf(t1, r);
            active.erase(r);
            dualSum += 1;  // y_r <- 1
        }
        // retroactive merge (line 76): skipped (no y change)
    }

    void proc4c(Label r1, Label r2, Label b)
    {
        Label trip[3] = {r1, r2, b};
        Label uHat = 0;
        for (Label x : trip)
        {
            Label o1 = 0, o2 = 0;
            for (Label y : trip)
                if (y != x)
                    (o1 == 0 ? o1 : o2) = y;
            if (not sameTree(t2, x, o1) && not sameTree(t2, x, o2))
            {
                uHat = x;
                break;
            }
        }
        if (uHat == 0)
            for (Label x : trip)
                if (aloneInTree(t2, x))
                {
                    uHat = x;
                    break;
                }
        if (uHat == 0)
            uHat = b;
        Label v1 = 0, v2 = 0;
        for (Label x : trip)
            if (x != uHat)
                (v1 == 0 ? v1 : v2) = x;
        const bool line80 = aloneInTree(t2, uHat);
        if (line80)
            cutOffLeaf(t1, uHat);
        else
        {
            cutOffLeaf(t2, uHat);
            cutOffLeaf(t1, uHat);
        }
        active.erase(uHat);
        dualSum += 1;  // y_uHat <- 1
        if (activeSiblings(t2, v1, v2))
        {
            if (v2 == b)
                std::swap(v1, v2);  // relabel so v1 = b (Claim 5)
            active.erase(v1);
            dualSum += 1;  // merge, y_b <- 1
        }
        else
        {
            resolvePair(v1, v2, {v1, v2});
            std::vector<Label> last;
            for (Label x : {v1, v2})
                if (active.contains(x))
                    last.push_back(x);
            if (not last.empty())
            {
                Label lv = last[0];
                if (not aloneInTree(t2, lv))
                    cutOffLeaf(t2, lv);
                cutOffLeaf(t1, lv);
                active.erase(lv);
                dualSum += 1;  // y_v2 <- 1
            }
        }
        // retroactive merge (line 91): skipped (no y change)
    }

    // ---- Preprocess (Procedure 3) ----
    void preprocess()
    {
        bool changed = true;
        while (changed)
        {
            changed = false;
            std::vector<Label> act(active.begin(), active.end());
            bool done = false;
            for (std::size_t i = 0; i < act.size() && not done; ++i)
                for (std::size_t j = i + 1; j < act.size(); ++j)
                {
                    Label u = act[i], v = act[j];
                    if (activeSiblings(t1, u, v) && activeSiblings(t2, u, v))
                    {
                        active.erase(u);  // merge, no dual change
                        changed = true;
                        done = true;
                        break;
                    }
                }
            if (done)
                continue;
            for (Label u : act)
            {
                if (active.size() <= 1)
                    break;
                if (active.contains(u) && aloneInTree(t2, u))
                {
                    cutOffLeaf(t1, u);
                    active.erase(u);
                    dualSum += 1;  // y_u <- 1
                    changed = true;
                    break;
                }
            }
        }
    }

    // ---- main loop (Algorithm 2) ----
    long run()
    {
        preprocess();
        long guard = 0;
        while (not active.empty())
        {
            if (++guard > 2000000)
                break;
            if (active.size() == 1)
            {
                active.clear();
                dualSum += 1;  // finalize last leaf
                break;
            }
            std::vector<Label> R, B;
            if (not findMinIncompat(R, B))
            {
                active.clear();  // remaining active leaves mutually compatible -> one tree
                dualSum += 1;
                break;
            }
            if (not resolveSet(R))  // Fail
            {
                // line 46: y_{lca1(RuB)} <- -1, y_{lca1(R)} <- +1  (net 0)
                const std::unordered_set<Label> Bset(B.begin(), B.end());
                while (static_cast<int>(activeIn(B).size()) >= 2)
                {
                    auto [a, b] = activeSiblingPairIn(B);
                    if (a == 0)
                        break;
                    resolvePair(a, b, Bset);
                }
                std::vector<Label> remR = activeIn(R);
                std::vector<Label> remB = activeIn(B);
                if (remR.size() == 2 && remB.size() == 1)
                {
                    Label r1 = remR[0], r2 = remR[1], b = remB[0];
                    std::unordered_set<Node*> roots{t2.rootOf(t2.node(r1)), t2.rootOf(t2.node(r2)),
                                                    t2.rootOf(t2.node(b))};
                    if (roots.size() == 1)
                        proc4a(r1, r2, b);
                    else if (roots.size() == 3)
                        proc4b(r1, r2, b);
                    else
                        proc4c(r1, r2, b);
                }
                else
                {
                    // degenerate remainder: finalize remaining active in R u B safely (each +1)
                    std::vector<Label> ru = R;
                    ru.insert(ru.end(), B.begin(), B.end());
                    for (Label x : ru)
                        if (active.contains(x))
                        {
                            if (not aloneInTree(t2, x))
                                cutOffLeaf(t2, x);
                            cutOffLeaf(t1, x);
                            active.erase(x);
                            dualSum += 1;
                        }
                }
            }
            preprocess();
        }
        return dualSum > 1 ? dualSum : 1;
    }
};
}  // namespace

namespace
{
/// The Red-Blue dual bound for the ordered forest pair (f1, f2). Cutting edges in f2 is charged, so the
/// value can differ per ordering; callers take the max over orderings for the tightest valid bound.
long redBlueTwoForest(const graph::Forest& f1, const graph::Forest& f2)
{
    ForestView t1w(f1), t2w(f2), t1o(f1), t2o(f2);
    ActiveSet active;
    active.reserve(t1w.nodeOf.size());
    for (const auto& [label, node] : t1w.nodeOf)
        active.insert(label);
    RedBlue2 rb(t1w, t2w, t1o, t2o, active);
    return rb.run();
}
}  // namespace

long solver::computeDual2ApproxLowerBound(const graph::Instance& instance)
{
    // A maximum agreement forest of all input forests is also an agreement forest of any pair of them,
    // so k*(all) >= k*(pair) >= L2(pair): the max over pairs is a valid lower bound for the full instance.
    const std::size_t k = instance.size();
    if (k < 2)
        return 1;
    long best = 1;
    for (std::size_t i = 0; i < k; ++i)
        for (std::size_t j = i + 1; j < k; ++j)
        {
            best = std::max(best, redBlueTwoForest(*instance.at(i), *instance.at(j)));
            best = std::max(best, redBlueTwoForest(*instance.at(j), *instance.at(i)));
        }
    return best;
}

long solver::computeCertifiedLowerBound(const graph::Instance& instance)
{
    // Both duals are always valid (<= k*); their maximum is valid too and never worse than either.
    const long l3 = computeDual3ApproxLowerBound(instance);
    const long l2 = computeDual2ApproxLowerBound(instance);
    return l2 > l3 ? l2 : l3;
}
