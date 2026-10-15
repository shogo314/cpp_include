#pragma once

/**
 * @file lowlink.hpp
 * @brief lowlink (関節点、橋、二重辺連結成分分解)
 */

#include <algorithm>
#include <cassert>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/graph/lowlink.hpp

/**
 * @brief 無向グラフの lowlink を求め、関節点・橋・二重辺連結成分を求める
 *
 * DFS を再帰を使わずに行う。O(n + m)
 * 多重辺と自己ループがあってもよい (親へ戻る辺は頂点ではなく辺の番号で区別する)
 * 例:
 * LowLink ll(g);
 * ll.articulation_points(); ll.bridges(); ll.two_edge_connected_groups();
 *
 * @tparam Cost 辺のコストの型
 */
template <typename Cost = int>
class LowLink {
    long long n;
    std::vector<bool> art, bridge;
    std::vector<long long> arts, bridge_ids;
    std::vector<int> comp;
    std::vector<std::vector<long long>> groups;

   public:
    std::vector<long long> ord;  //!< DFS で訪れた順番
    std::vector<long long> low;  //!< DFS 木の子孫から、木の辺を逆にたどらずに 1 本の後退辺で行ける頂点の ord の最小値

    explicit LowLink(const Graph<Cost>& graph) : n(graph.n), art(n), bridge(graph.m), comp(n, -1), ord(n, -1), low(n) {
        std::vector<int> parent(n, -1), parent_edge(n, -1), iter(n, 0);
        std::vector<long long> st;
        long long k = 0;
        for (long long root = 0; root < n; root++) {
            if (ord[root] != -1) continue;
            int root_children = 0;
            ord[root] = low[root] = k++;
            st.push_back(root);
            while (!st.empty()) {
                long long v = st.back();
                if (iter[v] < (int)graph[v].size()) {
                    const auto& e = graph[v][iter[v]++];
                    if (e.id == parent_edge[v]) continue;
                    long long u = e.dst;
                    if (ord[u] == -1) {
                        parent[u] = v;
                        parent_edge[u] = e.id;
                        ord[u] = low[u] = k++;
                        st.push_back(u);
                    } else {
                        low[v] = std::min(low[v], ord[u]);
                    }
                    continue;
                }
                st.pop_back();
                if (v == root) continue;
                long long p = parent[v];
                low[p] = std::min(low[p], low[v]);
                if (low[v] > ord[p]) bridge[parent_edge[v]] = true;
                if (p == root)
                    root_children++;
                else if (low[v] >= ord[p])
                    art[p] = true;
            }
            if (root_children >= 2) art[root] = true;
        }
        for (long long v = 0; v < n; v++) {
            if (art[v]) arts.push_back(v);
        }
        for (long long id = 0; id < graph.m; id++) {
            if (bridge[id]) bridge_ids.push_back(id);
        }
        // 橋を通らずに行ける頂点をまとめる
        for (long long s = 0; s < n; s++) {
            if (comp[s] != -1) continue;
            comp[s] = groups.size();
            groups.push_back({s});
            for (long long i = 0; i < (long long)groups.back().size(); i++) {
                long long v = groups.back()[i];
                for (const auto& e : graph[v]) {
                    if (bridge[e.id] || comp[e.dst] != -1) continue;
                    comp[e.dst] = comp[s];
                    groups.back().push_back(e.dst);
                }
            }
        }
    }

    /**
     * @brief 頂点数
     */
    long long size() const { return n; }

    /**
     * @brief v が関節点 (取り除くと連結成分が増える頂点) か
     */
    bool is_articulation_point(long long v) const {
        assert(0 <= v && v < n);
        return art[v];
    }
    /**
     * @brief 番号 id の辺が橋 (取り除くと連結成分が増える辺) か
     */
    bool is_bridge(long long id) const {
        assert(0 <= id && id < (long long)bridge.size());
        return bridge[id];
    }
    /**
     * @brief 関節点の頂点番号の昇順のリスト
     */
    const std::vector<long long>& articulation_points() const { return arts; }
    /**
     * @brief 橋の辺の番号の昇順のリスト
     */
    const std::vector<long long>& bridges() const { return bridge_ids; }
    /**
     * @brief 頂点 v が属する二重辺連結成分の番号
     */
    long long two_edge_connected_group_id(long long v) const {
        assert(0 <= v && v < n);
        return comp[v];
    }
    /**
     * @brief 二重辺連結成分 (橋を取り除いたときの連結成分) ごとの頂点のリスト
     */
    const std::vector<std::vector<long long>>& two_edge_connected_groups() const { return groups; }
};
