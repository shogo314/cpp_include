#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/link-cut-tree.hpp

/**
 * @file link-cut-tree.hpp
 * @brief Link-Cut Tree
 */

#include <cassert>
#include <utility>
#include <vector>

/**
 * @brief Link-Cut Tree (森で、辺の追加・削除とパス上の値の集約ができる)
 *
 * 各操作はならし O(log n)。内部の配列は int で持つ
 * 例:
 * LinkCutTree<monoid::Sum<long long>> lct(a);
 * lct.link(0, 1);
 * long long s = lct.prod(0, 1);
 *
 * @tparam M モノイド (algebra/monoid.hpp)。可換でなくてもよい (prod は u から v の向きにたたむ)
 */
template <typename M>
class LinkCutTree {
    using S = typename M::S;
    struct Node {
        int l = -1, r = -1, p = -1;
        bool rev = false;
        S val, sum, rsum;  // sum は左から右、rsum は右から左にたたんだもの
    };
    M m;
    S op(const S& a, const S& b) const { return m.op(a, b); }
    S e() const { return m.e(); }
    std::vector<Node> t;
    std::vector<int> stk;

    bool is_root(int x) const {
        int p = t[x].p;
        return p == -1 || (t[p].l != x && t[p].r != x);
    }
    S sum(int x) const { return x == -1 ? e() : t[x].sum; }
    S rsum(int x) const { return x == -1 ? e() : t[x].rsum; }
    void update(int x) {
        t[x].sum = op(op(sum(t[x].l), t[x].val), sum(t[x].r));
        t[x].rsum = op(op(rsum(t[x].r), t[x].val), rsum(t[x].l));
    }
    void toggle(int x) {
        if (x == -1) return;
        std::swap(t[x].l, t[x].r);
        std::swap(t[x].sum, t[x].rsum);
        t[x].rev = !t[x].rev;
    }
    void push(int x) {
        if (!t[x].rev) return;
        toggle(t[x].l);
        toggle(t[x].r);
        t[x].rev = false;
    }
    void rotate(int x) {
        int p = t[x].p, g = t[p].p;
        if (t[p].l == x) {
            t[p].l = t[x].r;
            if (t[x].r != -1) t[t[x].r].p = p;
            t[x].r = p;
        } else {
            t[p].r = t[x].l;
            if (t[x].l != -1) t[t[x].l].p = p;
            t[x].l = p;
        }
        t[p].p = x, t[x].p = g;
        if (g != -1) {
            if (t[g].l == p) t[g].l = x;
            if (t[g].r == p) t[g].r = x;
        }
        update(p), update(x);
    }
    void splay(int x) {
        for (int y = x;; y = t[y].p) {
            stk.push_back(y);
            if (is_root(y)) break;
        }
        while (!stk.empty()) push(stk.back()), stk.pop_back();
        while (!is_root(x)) {
            int p = t[x].p;
            if (!is_root(p)) rotate((t[t[p].p].l == p) == (t[p].l == x) ? p : x);
            rotate(x);
        }
    }
    // x から根までのパスを 1 本の splay 木にし、x をその根にする。最後に splay した頂点を返す
    int access(int x) {
        int last = -1;
        for (int y = x; y != -1; y = t[y].p) {
            splay(y);
            t[y].r = last;
            update(y);
            last = y;
        }
        splay(x);
        return last;
    }

   public:
    /**
     * @brief 頂点数 n、辺なし、値はすべて単位元
     *
     * @param m モノイド
     */
    explicit LinkCutTree(long long n = 0, M m = M()) : m(std::move(m)) {
        Node x;
        x.val = x.sum = x.rsum = e();
        t.assign(n, x);
    }
    /**
     * @brief 頂点 i の値が a[i]、辺なし
     */
    explicit LinkCutTree(const std::vector<S>& a, M m = M()) : LinkCutTree(a.size(), std::move(m)) {
        for (int i = 0; i < static_cast<int>(a.size()); i++) t[i].val = t[i].sum = t[i].rsum = a[i];
    }
    /**
     * @brief 頂点数
     */
    long long size() const { return static_cast<long long>(t.size()); }
    /**
     * @brief v を根にする
     */
    void evert(long long v) {
        access(v);
        toggle(v);
        push(v);
    }
    /**
     * @brief 辺 (u, v) を追加する。u と v は別の木にあること
     */
    void link(long long u, long long v) {
        assert(!connected(u, v));
        evert(u);
        t[u].p = static_cast<int>(v);
    }
    /**
     * @brief 辺 (u, v) を削除する。辺 (u, v) があること
     */
    void cut(long long u, long long v) {
        evert(u);
        access(v);
        assert(t[v].l == u && t[u].r == -1);
        t[v].l = t[u].p = -1;
        update(v);
    }
    /**
     * @brief v を含む木の根
     */
    long long root(long long v) {
        access(v);
        int x = static_cast<int>(v);
        while (push(x), t[x].l != -1) x = t[x].l;
        splay(x);
        return x;
    }
    /**
     * @brief u と v が同じ木にあるか
     */
    bool connected(long long u, long long v) { return u == v || root(u) == root(v); }
    /**
     * @brief u と v の最小共通祖先 (根は最後に evert した頂点など、今の根)。u と v は同じ木にあること
     */
    long long lca(long long u, long long v) {
        assert(connected(u, v));
        access(u);
        return access(v);
    }
    /**
     * @brief 頂点 v の値を x にする
     */
    void set(long long v, const S& x) {
        access(v);
        t[v].val = x;
        update(v);
    }
    /**
     * @brief 頂点 v の値
     */
    S get(long long v) const { return t[v].val; }
    /**
     * @brief 頂点 v の値を op(get(v), x) にする
     */
    void apply(long long v, const S& x) { set(v, op(t[v].val, x)); }
    /**
     * @brief u から v へのパス上の頂点 (端点を含む) の値を、u の側から順にたたんだもの。u と v は同じ木にあること
     *
     * u が根になる
     */
    S prod(long long u, long long v) {
        assert(connected(u, v));
        evert(u);
        access(v);
        return t[v].sum;
    }
};

/**
 * @brief 部分木の値の集約もできる Link-Cut Tree
 *
 * パスに含まれない子の部分木の値 (virtual) を持つ。各操作はならし O(log n)
 * 例:
 * SubtreeLinkCutTree<group::Sum<long long>> lct(a);
 * lct.link(0, 1);
 * long long s = lct.subtree_prod(1, 0);  // 0 を親としたときの 1 の部分木の和
 *
 * @tparam G 可換な群 (algebra/group.hpp)
 */

template <typename M>
LinkCutTree(long long, M) -> LinkCutTree<M>;
template <typename M>
LinkCutTree(std::vector<typename M::S>, M) -> LinkCutTree<M>;

template <typename G>
class SubtreeLinkCutTree {
    using S = typename G::S;
    struct Node {
        int l = -1, r = -1, p = -1;
        bool rev = false;
        S val, path, virt, sum;  // path はパス上、sum は virtual も含めた和
    };
    G g;
    S op(const S& a, const S& b) const { return g.op(a, b); }
    S e() const { return g.e(); }
    S inv(const S& a) const { return g.inv(a); }
    std::vector<Node> t;
    std::vector<int> stk;

    bool is_root(int x) const {
        int p = t[x].p;
        return p == -1 || (t[p].l != x && t[p].r != x);
    }
    S path(int x) const { return x == -1 ? e() : t[x].path; }
    S sum(int x) const { return x == -1 ? e() : t[x].sum; }
    void update(int x) {
        t[x].path = op(op(path(t[x].l), t[x].val), path(t[x].r));
        t[x].sum = op(op(sum(t[x].l), op(t[x].val, t[x].virt)), sum(t[x].r));
    }
    void toggle(int x) {
        if (x == -1) return;
        std::swap(t[x].l, t[x].r);
        t[x].rev = !t[x].rev;
    }
    void push(int x) {
        if (!t[x].rev) return;
        toggle(t[x].l);
        toggle(t[x].r);
        t[x].rev = false;
    }
    void rotate(int x) {
        int p = t[x].p, g = t[p].p;
        if (t[p].l == x) {
            t[p].l = t[x].r;
            if (t[x].r != -1) t[t[x].r].p = p;
            t[x].r = p;
        } else {
            t[p].r = t[x].l;
            if (t[x].l != -1) t[t[x].l].p = p;
            t[x].l = p;
        }
        t[p].p = x, t[x].p = g;
        if (g != -1) {
            if (t[g].l == p) t[g].l = x;
            if (t[g].r == p) t[g].r = x;
        }
        update(p), update(x);
    }
    void splay(int x) {
        for (int y = x;; y = t[y].p) {
            stk.push_back(y);
            if (is_root(y)) break;
        }
        while (!stk.empty()) push(stk.back()), stk.pop_back();
        while (!is_root(x)) {
            int p = t[x].p;
            if (!is_root(p)) rotate((t[t[p].p].l == p) == (t[p].l == x) ? p : x);
            rotate(x);
        }
    }
    int access(int x) {
        int last = -1;
        for (int y = x; y != -1; y = t[y].p) {
            splay(y);
            // 今の右の子は virtual になり、last は virtual でなくなる
            t[y].virt = op(op(t[y].virt, sum(t[y].r)), inv(sum(last)));
            t[y].r = last;
            update(y);
            last = y;
        }
        splay(x);
        return last;
    }

   public:
    explicit SubtreeLinkCutTree(long long n = 0, G g = G()) : g(std::move(g)) {
        Node x;
        x.val = x.path = x.virt = x.sum = e();
        t.assign(n, x);
    }
    explicit SubtreeLinkCutTree(const std::vector<S>& a, G g = G()) : SubtreeLinkCutTree(a.size(), std::move(g)) {
        for (int i = 0; i < static_cast<int>(a.size()); i++) t[i].val = t[i].path = t[i].sum = a[i];
    }
    long long size() const { return static_cast<long long>(t.size()); }
    void evert(long long v) {
        access(v);
        toggle(v);
        push(v);
    }
    void link(long long u, long long v) {
        assert(!connected(u, v));
        evert(u);
        access(v);
        t[u].p = static_cast<int>(v);
        t[v].virt = op(t[v].virt, t[u].sum);
        update(v);
    }
    void cut(long long u, long long v) {
        evert(u);
        access(v);
        assert(t[v].l == u && t[u].r == -1);
        t[v].l = t[u].p = -1;
        update(v);
    }
    long long root(long long v) {
        access(v);
        int x = static_cast<int>(v);
        while (push(x), t[x].l != -1) x = t[x].l;
        splay(x);
        return x;
    }
    bool connected(long long u, long long v) { return u == v || root(u) == root(v); }
    long long lca(long long u, long long v) {
        assert(connected(u, v));
        access(u);
        return access(v);
    }
    void set(long long v, const S& x) {
        access(v);
        t[v].val = x;
        update(v);
    }
    S get(long long v) const { return t[v].val; }
    void apply(long long v, const S& x) { set(v, op(t[v].val, x)); }
    /**
     * @brief u から v へのパス上の頂点 (端点を含む) の値の和。u と v は同じ木にあること
     */
    S prod(long long u, long long v) {
        assert(connected(u, v));
        evert(u);
        access(v);
        return t[v].path;
    }
    /**
     * @brief p を根としたときの v の部分木の値の和 (p == v なら木全体)。v と p は同じ木にあること
     *
     * p が根になる
     */
    S subtree_prod(long long v, long long p) {
        assert(connected(v, p));
        evert(p);
        access(v);
        return op(t[v].val, t[v].virt);
    }
};

template <typename G>
SubtreeLinkCutTree(long long, G) -> SubtreeLinkCutTree<G>;
template <typename G>
SubtreeLinkCutTree(std::vector<typename G::S>, G) -> SubtreeLinkCutTree<G>;
