#pragma once

/**
 * @file heavy-light-decomposition.hpp
 * @brief HL 分解 (Heavy-Light Decomposition)
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/main/src/graph/heavy-light-decomposition.hpp

/**
 * @brief HL 分解 (Heavy-Light Decomposition)
 *
 * 木の頂点を、どのパスも O(log n) 個の区間に分かれ、どの部分木も 1 つの区間になるように並べ替える
 * 頂点 v は列の index(v) 番目に対応する。セグメント木などと組み合わせて、パスや部分木へのクエリを行う
 * 前計算 O(n)。DFS は再帰を使わない
 * 例:
 * HeavyLightDecomposition hld(tree, root);  // tree は Tree<Cost>
 * for (auto [a, b] : hld.path(u, v)) sum += seg.prod(std::min(a, b), std::max(a, b) + 1);
 * auto [l, r] = hld.subtree(v);  // 部分木は区間 [l, r)
 */
class HeavyLightDecomposition {
    int n, root;
    std::vector<int> par, dep, sz, heavy, head, in, order;

   public:
    /**
     * @brief 木 tree を根 root で HL 分解する
     */
    template <typename Cost>
    explicit HeavyLightDecomposition(const Tree<Cost>& tree, int root = 0)
        : n(tree.size()), root(root), par(n, -1), dep(n, 0), sz(n, 1), heavy(n, -1), head(n), in(n), order(n) {
        assert(0 <= root && root < n);
        // 根からの BFS 順で親と深さを決める
        std::vector<int> bfs = {root};
        bfs.reserve(n);
        for (int i = 0; i < (int)bfs.size(); i++) {
            int v = bfs[i];
            for (int u : tree[v]) {
                if (u == par[v]) continue;
                assert(u != root && par[u] == -1 && "HeavyLightDecomposition: not a tree");
                par[u] = v;
                dep[u] = dep[v] + 1;
                bfs.push_back(u);
            }
        }
        assert((int)bfs.size() == n && "HeavyLightDecomposition: not connected");
        // 葉の側から部分木の大きさを求め、最も大きい子を heavy にする
        for (int i = n - 1; i > 0; i--) {
            int v = bfs[i], p = par[v];
            sz[p] += sz[v];
            if (heavy[p] == -1 || sz[v] > sz[heavy[p]]) heavy[p] = v;
        }
        // heavy な子を最後に積むと、次に取り出されて index が連続する
        std::vector<int> st = {root};
        head[root] = root;
        int k = 0;
        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            in[v] = k;
            order[k++] = v;
            for (int u : tree[v]) {
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

    int size() const { return n; }
    /**
     * @brief 頂点 v の列での位置
     */
    int index(int v) const { return in[v]; }
    /**
     * @brief 列の i 番目の頂点 (index の逆)
     */
    int vertex(int i) const { return order[i]; }
    /**
     * @brief v の親 (根なら -1)
     */
    int parent(int v) const { return par[v]; }
    /**
     * @brief v の深さ (根は 0、辺の本数)
     */
    int depth(int v) const { return dep[v]; }
    /**
     * @brief v を根とする部分木の頂点の個数
     */
    int subtree_size(int v) const { return sz[v]; }
    /**
     * @brief v を根とする部分木の頂点に対応する区間 [l, r)
     */
    std::pair<int, int> subtree(int v) const { return {in[v], in[v] + sz[v]}; }

    /**
     * @brief u と v の最小共通祖先 O(log n)
     */
    int lca(int u, int v) const {
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
    int dist(int u, int v) const { return dep[u] + dep[v] - 2 * dep[lca(u, v)]; }
    /**
     * @brief v から根の方へ k 本の辺を進んだ頂点 (k > depth(v) なら -1) O(log n)
     */
    int hop(int v, int k) const {
        assert(0 <= v && v < n && k >= 0);
        if (k > dep[v]) return -1;
        while (true) {
            int h = head[v];
            if (in[v] - in[h] >= k) return order[in[v] - k];
            k -= in[v] - in[h] + 1;
            v = par[h];
        }
    }
    /**
     * @brief u から v へのパスで、u から k 本の辺を進んだ頂点 (k > dist(u, v) なら -1) O(log n)
     */
    int jump(int u, int v, int k) const {
        assert(0 <= u && u < n && 0 <= v && v < n && k >= 0);
        int w = lca(u, v);
        int du = dep[u] - dep[w], dv = dep[v] - dep[w];
        if (k <= du) return hop(u, k);
        if (k <= du + dv) return hop(v, du + dv - k);
        return -1;
    }

    /**
     * @brief u から v へのパスを、列の区間に分けて u 側から順に並べる O(log n)
     *
     * 各要素 (a, b) は、列の a 番目から b 番目まで (両端を含む) を、a から b の向きにたどることを表す
     * 向きを表すため、このライブラリの他の区間 (半開区間 [l, r)) と違って閉区間になっている
     * a > b なら index が減る向き (根に近づく向き) にたどる
     * 交換法則が成り立たない演算では、a > b の区間は逆順の積を使う
     * @param edge true なら辺についてのパスにする。辺 (parent(v), v) を index(v) 番目に対応させ、lca の位置を含めない
     */
    std::vector<std::pair<int, int>> path(int u, int v, bool edge = false) const {
        assert(0 <= u && u < n && 0 <= v && v < n);
        std::vector<std::pair<int, int>> up, down;
        while (head[u] != head[v]) {
            if (dep[head[u]] >= dep[head[v]]) {
                up.emplace_back(in[u], in[head[u]]);
                u = par[head[u]];
            } else {
                down.emplace_back(in[head[v]], in[v]);
                v = par[head[v]];
            }
        }
        if (dep[u] >= dep[v]) {
            // v が lca
            int l = in[v] + edge;
            if (l <= in[u]) up.emplace_back(in[u], l);
        } else {
            // u が lca
            int l = in[u] + edge;
            if (l <= in[v]) down.emplace_back(l, in[v]);
        }
        up.insert(up.end(), down.rbegin(), down.rend());
        return up;
    }
};
