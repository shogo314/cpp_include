#pragma once

/**
 * @file dominator-tree.hpp
 * @brief 支配木
 */

#include <cassert>
#include <utility>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/graph/dominator-tree.hpp

/**
 * @brief 有向グラフの支配木 (Lengauer–Tarjan) O(m log n)
 *
 * root から v へのどの道も通る頂点のうち、v 以外で v に最も近いもの (直近支配点) を求める
 * @return 各頂点の直近支配点。root と、root から到達できない頂点は -1
 */
template <typename Cost>
std::vector<long long> dominator_tree(const Graph<Cost>& graph, long long root) {
    const long long n = graph.n;
    assert(0 <= root && root < n);
    std::vector<std::vector<long long>> radj(n);
    for (long long u = 0; u < n; u++) {
        for (const auto& e : graph[u]) radj[e.dst].push_back(u);
    }
    // DFS の行きがけ順。以下は頂点ではなく、この順番 (0 が root) で扱う
    std::vector<long long> ord(n, -1), vertex, par;
    {
        std::vector<std::pair<long long, long long>> stk = {{root, -1}};
        while (!stk.empty()) {
            auto [u, p] = stk.back();
            stk.pop_back();
            if (ord[u] != -1) continue;
            ord[u] = vertex.size();
            vertex.push_back(u);
            par.push_back(p);
            for (const auto& e : graph[u]) {
                if (ord[e.dst] == -1) stk.emplace_back(e.dst, ord[u]);
            }
        }
    }
    const long long k = vertex.size();
    // semi: 準支配点、anc・label: 処理済みの森 (経路圧縮つき) で semi が最小の頂点を求めるためのもの
    std::vector<long long> semi(k), idom(k, -1), anc(k, -1), label(k);
    std::vector<std::vector<long long>> bucket(k);
    for (long long i = 0; i < k; i++) semi[i] = label[i] = i;
    std::vector<long long> path;
    // v から森の根の手前までで、semi が最小の頂点
    auto eval = [&](long long v) {
        if (anc[v] == -1) return v;
        for (long long x = v; anc[anc[x]] != -1; x = anc[x]) path.push_back(x);
        // 根に近い方から圧縮する
        while (!path.empty()) {
            long long x = path.back();
            path.pop_back();
            if (semi[label[anc[x]]] < semi[label[x]]) label[x] = label[anc[x]];
            anc[x] = anc[anc[x]];
        }
        return label[v];
    };
    for (long long w = k - 1; w >= 1; w--) {
        for (long long u : radj[vertex[w]]) {
            if (ord[u] == -1) continue;
            semi[w] = std::min(semi[w], semi[eval(ord[u])]);
        }
        bucket[semi[w]].push_back(w);
        anc[w] = par[w];
        for (long long v : bucket[par[w]]) {
            long long u = eval(v);
            idom[v] = semi[u] < semi[v] ? u : par[w];
        }
        bucket[par[w]].clear();
    }
    for (long long w = 1; w < k; w++) {
        if (idom[w] != semi[w]) idom[w] = idom[idom[w]];
    }
    std::vector<long long> res(n, -1);
    for (long long w = 1; w < k; w++) res[vertex[w]] = vertex[idom[w]];
    return res;
}
