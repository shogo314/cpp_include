#pragma once

/**
 * @file lowlink.hpp
 * @brief lowlink (関節点、橋、二重辺連結成分分解、二重頂点連結成分分解、Block-Cut Tree)
 */

#include <algorithm>
#include <cassert>
#include <vector>

#include "graph.hpp"
#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/graph/lowlink.hpp

/**
 * @brief 無向グラフの lowlink を求め、関節点・橋・二重辺連結成分・二重頂点連結成分を求める
 *
 * DFS を再帰を使わずに行う。O(n + m)
 * 多重辺と自己ループがあってもよい (親へ戻る辺は頂点ではなく辺の番号で区別する)
 * 自己ループはどの二重頂点連結成分の辺にも入れない
 * 例:
 * LowLink ll(g);
 * ll.articulation_points(); ll.bridges(); ll.two_edge_connected_groups();
 * ll.two_vertex_connected_groups(); ll.block_cut_tree();
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
    std::vector<std::vector<long long>> bc_groups, bc_edge_groups;
    std::vector<long long> bc_edge_id;

   public:
    std::vector<long long> ord;  //!< DFS で訪れた順番
    std::vector<long long> low;  //!< DFS 木の子孫から、木の辺を逆にたどらずに 1 本の後退辺で行ける頂点の ord の最小値

    explicit LowLink(const Graph<Cost>& graph) : n(graph.n), art(n), bridge(graph.m), comp(n, -1), bc_edge_id(graph.m, -1), ord(n, -1), low(n) {
        std::vector<int> parent(n, -1), parent_edge(n, -1), iter(n, 0);
        std::vector<long long> st;
        // 二重頂点連結成分を作るために、通った辺 (木の辺と、子孫から祖先への後退辺) を積む
        std::vector<long long> edge_st, edge_src(graph.m), edge_dst(graph.m), last_block(n, -1);
        auto push_edge = [&](long long id, long long a, long long b) {
            edge_st.push_back(id);
            edge_src[id] = a, edge_dst[id] = b;
        };
        long long k = 0;
        for (long long root = 0; root < n; root++) {
            if (ord[root] != -1) continue;
            int root_children = 0;
            ord[root] = low[root] = k++;
            st.push_back(root);
            while (!st.empty()) {
                long long v = st.back();
                if (iter[v] < static_cast<int>(graph[v].size())) {
                    const auto& e = graph[v][iter[v]++];
                    if (e.id == parent_edge[v]) continue;
                    long long u = e.dst;
                    if (ord[u] == -1) {
                        parent[u] = v;
                        parent_edge[u] = e.id;
                        ord[u] = low[u] = k++;
                        st.push_back(u);
                        push_edge(e.id, v, u);
                    } else {
                        low[v] = std::min(low[v], ord[u]);
                        // 後退辺は子孫の側から見たときに 1 回だけ積む (自己ループは積まない)
                        if (ord[u] < ord[v]) push_edge(e.id, v, u);
                    }
                    continue;
                }
                st.pop_back();
                if (v == root) continue;
                long long p = parent[v];
                low[p] = std::min(low[p], low[v]);
                if (low[v] > ord[p]) bridge[parent_edge[v]] = true;
                if (low[v] >= ord[p]) {
                    // 親との辺までが 1 つの二重頂点連結成分
                    long long b = bc_groups.size();
                    bc_groups.emplace_back();
                    bc_edge_groups.emplace_back();
                    while (true) {
                        long long id = edge_st.back();
                        edge_st.pop_back();
                        bc_edge_id[id] = b;
                        bc_edge_groups[b].push_back(id);
                        for (long long w : {edge_src[id], edge_dst[id]}) {
                            if (last_block[w] != b) last_block[w] = b, bc_groups[b].push_back(w);
                        }
                        if (id == parent_edge[v]) break;
                    }
                }
                if (p == root)
                    root_children++;
                else if (low[v] >= ord[p])
                    art[p] = true;
            }
            if (root_children >= 2) art[root] = true;
            // ほかの頂点とつながっていない頂点は、それだけで 1 つの二重頂点連結成分
            if (root_children == 0) {
                bc_groups.push_back({root});
                bc_edge_groups.emplace_back();
            }
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
            for (long long i = 0; i < static_cast<long long>(groups.back().size()); i++) {
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
        assert(0 <= id && id < static_cast<long long>(bridge.size()));
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

    /**
     * @brief 二重頂点連結成分ごとの頂点のリスト
     * 関節点は複数の成分に入る。ほかの頂点とつながっていない頂点は、それだけで 1 つの成分になる
     */
    const std::vector<std::vector<long long>>& two_vertex_connected_groups() const { return bc_groups; }
    /**
     * @brief 二重頂点連結成分ごとの辺の番号のリスト (two_vertex_connected_groups() と同じ順)
     * 自己ループ以外の辺はちょうど 1 つの成分に入る
     */
    const std::vector<std::vector<long long>>& two_vertex_connected_edge_groups() const { return bc_edge_groups; }
    /**
     * @brief 番号 id の辺が属する二重頂点連結成分の番号 (自己ループなら -1)
     */
    long long two_vertex_connected_group_id(long long id) const {
        assert(0 <= id && id < static_cast<long long>(bc_edge_id.size()));
        return bc_edge_id[id];
    }
    /**
     * @brief Block-Cut Tree
     * 頂点 0, ..., n - 1 は元の頂点、頂点 n + b は二重頂点連結成分 b で、元の頂点と、それを含む成分を辺で結ぶ
     * 元のグラフが連結なら木、そうでなければ森になる (森のときは、連結であることを前提とする木の関数には渡せない)
     */
    Tree<int> block_cut_tree() const {
        Tree<int> res(n + static_cast<long long>(bc_groups.size()));
        for (long long b = 0; b < static_cast<long long>(bc_groups.size()); b++) {
            for (long long v : bc_groups[b]) res.add_edge(v, n + b);
        }
        return res;
    }
};
