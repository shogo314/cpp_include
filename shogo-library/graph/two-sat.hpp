#pragma once

/**
 * @file two-sat.hpp
 * @brief 2-SAT
 */

#include <cassert>
#include <vector>

#include "graph.hpp"
#include "graph_util.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/graph/two-sat.hpp

/**
 * @brief 2-SAT
 *
 * 変数 x_0, ..., x_{n-1} についての「(x_i = f) または (x_j = g)」という形の条件を並べて、
 * すべてを満たす割り当てがあるかを判定する
 * 強連結成分分解を使う。O(n + 条件の個数)
 * 例:
 * TwoSat ts(n);
 * ts.add_clause(0, true, 1, false);  // x_0 または (not x_1)
 * if (ts.satisfiable()) auto ans = ts.answer();
 */
class TwoSat {
    long long n;
    // 頂点 i は「x_i = true」、頂点 n + i は「x_i = false」を表す
    Graph<int> graph;
    std::vector<bool> ans;
    long long literal(long long i, bool f) const { return f ? i : n + i; }

   public:
    explicit TwoSat(long long n = 0) : n(n), graph(2 * n), ans(n) {}

    long long size() const { return n; }

    /**
     * @brief 条件「(x_i = f) または (x_j = g)」を加える
     */
    void add_clause(long long i, bool f, long long j, bool g) {
        assert(0 <= i && i < n && 0 <= j && j < n);
        // (x_i != f) なら (x_j = g)、(x_j != g) なら (x_i = f)
        graph.add_directed_edge(literal(i, !f), literal(j, g));
        graph.add_directed_edge(literal(j, !g), literal(i, f));
    }
    /**
     * @brief 条件「(x_i = f) ならば (x_j = g)」を加える
     */
    void add_imply(long long i, bool f, long long j, bool g) { add_clause(i, !f, j, g); }
    /**
     * @brief 条件「x_i = f」を加える
     */
    void add_fix(long long i, bool f) { add_clause(i, f, i, f); }
    /**
     * @brief 条件「(x_i = f) と (x_j = g) の少なくとも一方は成り立たない」を加える
     */
    void add_nand(long long i, bool f, long long j, bool g) { add_clause(i, !f, j, !g); }

    /**
     * @brief すべての条件を満たす割り当てがあるか。あれば answer() で 1 つ得られる
     */
    bool satisfiable() {
        auto groups = strongly_connected_components(graph);
        // 成分のトポロジカル順での番号
        std::vector<int> id(2 * n);
        for (int k = 0; k < static_cast<int>(groups.size()); k++) {
            for (long long v : groups[k]) id[v] = k;
        }
        for (long long i = 0; i < n; i++) {
            if (id[i] == id[n + i]) return false;
            // トポロジカル順で後ろにある方を真にする
            ans[i] = id[i] > id[n + i];
        }
        return true;
    }
    /**
     * @brief 直前の satisfiable() が true のとき、条件を満たす割り当て
     */
    const std::vector<bool>& answer() const { return ans; }
};
