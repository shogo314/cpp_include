#pragma once

/**
 * @file heavy-light-decomposition.hpp
 * @brief HL 分解 (Heavy-Light Decomposition)
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/adb06bae4b0e202b3a1fe53ef9f9a1b30077d8cc/src/graph/heavy-light-decomposition.hpp

/**
 * @brief HL 分解 (Heavy-Light Decomposition)
 *
 * 木の頂点を、どのパスも O(log n) 個の区間に分かれ、どの部分木も 1 つの区間になるように並べ替える
 * 頂点 v は列の index(v) 番目に対応する。セグメント木などと組み合わせて、パスや部分木へのクエリを行う
 * 前計算 O(n)。DFS は再帰を使わない
 * 例:
 * HeavyLightDecomposition hld(tree, root);  // tree は Tree<Cost>
 * for (auto [l, r, reverse] : hld.path(u, v)) sum += seg.prod(l, r);
 * auto [l, r] = hld.subtree(v);  // 部分木は区間 [l, r)
 */
class HeavyLightDecomposition {
    long long n, root;
    // 内部の配列は int で持ち、公開する関数で long long にする
    std::vector<int> par, dep, sz, heavy, head, in, order;

   public:
    /**
     * @brief パスの一部に対応する半開区間 [l, r) と、たどる向き
     */
    struct PathSegment {
        long long l;   //!< 区間の左端 (含む)
        long long r;   //!< 区間の右端 (含まない)
        bool reverse;  //!< true なら r - 1 から l へ、false なら l から r - 1 へたどる
    };

    /**
     * @brief 木 tree を根 root で HL 分解する
     */
    template <typename Cost>
    explicit HeavyLightDecomposition(const Tree<Cost>& tree, long long root = 0)
        : n(tree.size()), root(root), par(n, -1), dep(n, 0), sz(n, 1), heavy(n, -1), head(n), in(n), order(n) {
        assert(0 <= root && root < n);
        // 根からの BFS 順で親と深さを決める
        std::vector<int> bfs = {static_cast<int>(root)};
        bfs.reserve(n);
        for (int i = 0; i < static_cast<int>(bfs.size()); i++) {
            int v = bfs[i];
            for (long long u : tree[v]) {
                if (u == par[v]) continue;
                assert(u != root && par[u] == -1 && "HeavyLightDecomposition: not a tree");
                par[u] = v;
                dep[u] = dep[v] + 1;
                bfs.push_back(u);
            }
        }
        assert(static_cast<long long>(bfs.size()) == n && "HeavyLightDecomposition: not connected");
        // 葉の側から部分木の大きさを求め、最も大きい子を heavy にする
        for (int i = n - 1; i > 0; i--) {
            int v = bfs[i], p = par[v];
            sz[p] += sz[v];
            if (heavy[p] == -1 || sz[v] > sz[heavy[p]]) heavy[p] = v;
        }
        // heavy な子を最後に積むと、次に取り出されて index が連続する
        std::vector<int> st = {static_cast<int>(root)};
        head[root] = root;
        int k = 0;
        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            in[v] = k;
            order[k++] = v;
            for (long long u : tree[v]) {
                if (u == par[v] || u == heavy[v]) continue;
                head[u] = u;
                st.push_back(u);
            }
            if (heavy[v] != -1) {
                head[heavy[v]] = head[v];
                st.push_back(heavy[v]);
            }
        }
    }

    long long size() const { return n; }
    /**
     * @brief 頂点 v の列での位置
     */
    long long index(long long v) const {
        assert(0 <= v && v < n);
        return in[v];
    }
    /**
     * @brief 列の i 番目の頂点 (index の逆)
     */
    long long vertex(long long i) const {
        assert(0 <= i && i < n);
        return order[i];
    }
    /**
     * @brief v の親 (根なら -1)
     */
    long long parent(long long v) const {
        assert(0 <= v && v < n);
        return par[v];
    }
    /**
     * @brief v の深さ (根は 0、辺の本数)
     */
    long long depth(long long v) const {
        assert(0 <= v && v < n);
        return dep[v];
    }
    /**
     * @brief v を根とする部分木の頂点の個数
     */
    long long subtree_size(long long v) const {
        assert(0 <= v && v < n);
        return sz[v];
    }
    /**
     * @brief v を根とする部分木の頂点に対応する区間 [l, r)
     */
    std::pair<long long, long long> subtree(long long v) const { return {in[v], in[v] + sz[v]}; }

    /**
     * @brief u と v の最小共通祖先 O(log n)
     */
    long long lca(long long u, long long v) const {
        assert(0 <= u && u < n && 0 <= v && v < n);
        while (head[u] != head[v]) {
            if (dep[head[u]] < dep[head[v]]) std::swap(u, v);
            u = par[head[u]];
        }
        return dep[u] < dep[v] ? u : v;
    }
    /**
     * @brief u と v の間の辺の本数 O(log n)
     */
    long long dist(long long u, long long v) const { return dep[u] + dep[v] - 2 * dep[lca(u, v)]; }
    /**
     * @brief v から根の方へ k 本の辺を進んだ頂点 (k > depth(v) なら -1) O(log n)
     */
    long long hop(long long v, long long k) const {
        assert(0 <= v && v < n && k >= 0);
        if (k > dep[v]) return -1;
        while (true) {
            long long h = head[v];
            if (in[v] - in[h] >= k) return order[in[v] - k];
            k -= in[v] - in[h] + 1;
            v = par[h];
        }
    }
    /**
     * @brief u から v へのパスで、u から k 本の辺を進んだ頂点 (k > dist(u, v) なら -1) O(log n)
     */
    long long jump(long long u, long long v, long long k) const {
        assert(0 <= u && u < n && 0 <= v && v < n && k >= 0);
        long long w = lca(u, v);
        long long du = dep[u] - dep[w], dv = dep[v] - dep[w];
        if (k <= du) return hop(u, k);
        if (k <= du + dv) return hop(v, du + dv - k);
        return -1;
    }

   private:
    class PathEnumeration {
        const HeavyLightDecomposition* hld;
        int u, v;
        bool edge;

       public:
        PathEnumeration(const HeavyLightDecomposition* hld, long long u, long long v, bool edge)
            : hld(hld), u(static_cast<int>(u)), v(static_cast<int>(v)), edge(edge) {}

        class iterator {
            struct Interval {
                int l, r;
            };
            const HeavyLightDecomposition* hld = nullptr;
            int u = 0, v = 0, top = 0;
            bool edge = false, ascending = true, done = true;
            // v 側から集めた区間を逆順に返す。light edge を越えるたびに部分木の大きさは倍以上になる。
            // 未使用の要素は初期化せず、iterator のコピーでも [0, top) だけをコピーする。
            std::array<Interval, std::numeric_limits<long long>::digits> pending;
            PathSegment current{0, 0, false};

           public:
            using value_type = PathSegment;
            using difference_type = std::ptrdiff_t;
            using reference = value_type;
            using pointer = void;
            using iterator_category = std::input_iterator_tag;

            iterator() = default;
            iterator(const HeavyLightDecomposition* hld, long long u, long long v, bool edge)
                : hld(hld), u(static_cast<int>(u)), v(static_cast<int>(v)), edge(edge), done(false) { ++*this; }
            iterator(const iterator& other)
                : hld(other.hld), u(other.u), v(other.v), top(other.top), edge(other.edge), ascending(other.ascending), done(other.done), current(other.current) {
                std::copy_n(other.pending.begin(), top, pending.begin());
            }
            iterator& operator=(const iterator& other) {
                if (this != &other) {
                    hld = other.hld;
                    u = other.u;
                    v = other.v;
                    top = other.top;
                    edge = other.edge;
                    ascending = other.ascending;
                    done = other.done;
                    current = other.current;
                    std::copy_n(other.pending.begin(), top, pending.begin());
                }
                return *this;
            }
            value_type operator*() const { return current; }
            iterator& operator++() {
                if (ascending) {
                    while (hld->head[u] != hld->head[v]) {
                        if (hld->dep[hld->head[u]] >= hld->dep[hld->head[v]]) {
                            current = {hld->in[hld->head[u]], hld->in[u] + 1, true};
                            u = hld->par[hld->head[u]];
                            return *this;
                        }
                        pending[top++] = {hld->in[hld->head[v]], hld->in[v] + 1};
                        v = hld->par[hld->head[v]];
                    }
                    ascending = false;
                    if (hld->dep[u] >= hld->dep[v]) {
                        const long long l = hld->in[v] + edge;
                        if (l <= hld->in[u]) {
                            current = {l, hld->in[u] + 1, true};
                            return *this;
                        }
                    } else {
                        const long long l = hld->in[u] + edge;
                        if (l <= hld->in[v]) {
                            current = {l, hld->in[v] + 1, false};
                            return *this;
                        }
                    }
                }
                if (top > 0) {
                    const auto [l, r] = pending[--top];
                    current = {l, r, false};
                } else {
                    done = true;
                }
                return *this;
            }
            // 後置 increment では、退避した区間をコピーせず、変更前の値だけを返す。
            struct Postfix {
                value_type value;
                value_type operator*() const { return value; }
            };
            Postfix operator++(int) {
                const Postfix previous{current};
                ++*this;
                return previous;
            }
            bool operator==(const iterator& other) const {
                if (done || other.done) return done == other.done;
                return hld == other.hld && current.l == other.current.l && current.r == other.current.r && current.reverse == other.current.reverse;
            }
            bool operator!=(const iterator& other) const { return !(*this == other); }
        };

        iterator begin() const { return iterator(hld, u, v, edge); }
        iterator end() const { return iterator(); }
    };

   public:
    /**
     * @brief u から v へのパスの区間を u 側から順に遅延列挙する。構築 O(1)、全走査 O(log n)
     *
     * 各要素は半開区間 [l, r) と向き reverse を持つ PathSegment
     * reverse が false なら index の昇順、true なら降順 (根に近づく向き) にたどる
     * 交換法則が成り立たない演算では、reverse が true の区間は逆順の積を使う
     * @pre 走査中は、この HeavyLightDecomposition を有効なまま保持し、変更しないこと
     * @param edge true なら辺についてのパスにする。辺 (parent(v), v) を index(v) 番目に対応させ、lca の位置を含めない
     */
    auto path(long long u, long long v, bool edge = false) const {
        assert(0 <= u && u < n && 0 <= v && v < n);
        return PathEnumeration(this, u, v, edge);
    }
};
