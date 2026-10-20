#pragma once

/**
 * @file auxiliary-tree.hpp
 * @brief Auxiliary Tree (虚树、指定した頂点とその LCA だけを残して圧縮した木)
 */

#include <algorithm>
#include <cassert>
#include <vector>

#include "heavy-light-decomposition.hpp"
#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/graph/auxiliary-tree.hpp

/**
 * @brief Auxiliary Tree (指定した頂点とその LCA だけを残して圧縮した木)
 *
 * k 個の頂点を指定すると、それらと、それらの LCA だけを頂点とする木 (頂点は 2k - 1 個以下) を O(k log n) で作る
 * 圧縮した木の辺は、元の木のパスを 1 本の辺にまとめたもの。LCA や距離は hld() で求める
 * 例:
 * AuxiliaryTree aux(tree, 0);
 * auto t = aux.build({3, 5, 8});
 * // t.vertices[i] の親は t.vertices[t.parent[i]] (根は t.parent[0] == -1)
 * for (long long i = t.size() - 1; i > 0; i--) dp[t.parent[i]] += dp[i];  // 子から親への DP
 */
class AuxiliaryTree {
    HeavyLightDecomposition hld_;

   public:
    /**
     * @brief build の戻り値
     * vertices は HL 分解の列の順 (DFS の行きがけ順の 1 つ) に並ぶので、parent[i] < i (根は 0 番目で parent[0] = -1)
     */
    struct Result {
        std::vector<long long> vertices;  //!< 圧縮した木の頂点 (元の木の頂点番号)
        std::vector<long long> parent;    //!< 親の、vertices の中での添字 (根は -1)
        long long size() const { return vertices.size(); }
    };

    /**
     * @brief 木 tree を根 root で前計算する O(n)
     */
    template <typename Cost>
    explicit AuxiliaryTree(const Tree<Cost>& tree, long long root = 0) : hld_(tree, root) {}

    /**
     * @brief 頂点 vs (重複してもよい) とその LCA だけを残した木を作る O(k log n) (k は vs の個数)
     * vs が空なら、頂点のない木を返す
     */
    Result build(std::vector<long long> vs) const {
        auto by_order = [&](long long a, long long b) { return hld_.index(a) < hld_.index(b); };
        std::sort(vs.begin(), vs.end(), by_order);
        vs.erase(std::unique(vs.begin(), vs.end()), vs.end());
        // DFS 順で隣り合う頂点の LCA を足すと、LCA について閉じる
        const long long k = vs.size();
        for (long long i = 0; i + 1 < k; i++) vs.push_back(hld_.lca(vs[i], vs[i + 1]));
        std::sort(vs.begin(), vs.end(), by_order);
        vs.erase(std::unique(vs.begin(), vs.end()), vs.end());
        Result res;
        res.vertices = vs;
        res.parent.assign(vs.size(), -1);
        // DFS 順で 1 つ前の頂点との LCA が親になる
        for (long long i = 1; i < static_cast<long long>(vs.size()); i++) {
            long long p = hld_.lca(vs[i - 1], vs[i]);
            res.parent[i] = std::lower_bound(vs.begin(), vs.end(), p, by_order) - vs.begin();
        }
        return res;
    }

    /**
     * @brief 前計算に使った HL 分解 (LCA や距離、深さを求めるのに使う)
     */
    const HeavyLightDecomposition& hld() const { return hld_; }
};
