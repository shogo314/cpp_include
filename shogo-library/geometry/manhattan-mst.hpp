#pragma once

/**
 * @file manhattan-mst.hpp
 * @brief マンハッタン距離の最小全域木
 */

#include <algorithm>
#include <map>
#include <numeric>
#include <vector>

#include "../graph/graph_util.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/geometry/manhattan-mst.hpp

/**
 * @brief 点の集合で、2 点を結ぶ辺の重みをマンハッタン距離 |dx| + |dy| とした完全グラフの最小全域木 O(n log n)
 *
 * 各点 i について、i を頂点とする 45 度ずつの 8 つの領域のそれぞれで最も近い点への辺だけを候補にすればよい。
 * 向きを変えながら 4 回掃いて O(n) 本の候補を作り、Kruskal 法にかける
 * @return 最小全域木 (辺の src、dst は点の番号、cost は距離)
 */
template <typename T>
MinimumSpanningTree<T> manhattan_mst(std::vector<Vector<T, 2>> p) {
    const long long n = p.size();
    Graph<T> g(n);
    std::vector<long long> id(n);
    std::iota(id.begin(), id.end(), 0);
    for (int k = 0; k < 4; k++) {
        // x + y の昇順に見て、x - y の条件を満たす中で y が近い点を map から取り出す
        std::sort(id.begin(), id.end(), [&](long long i, long long j) { return p[i][0] + p[i][1] < p[j][0] + p[j][1]; });
        std::map<T, long long> sweep;  // -y -> 点
        for (long long i : id) {
            for (auto it = sweep.lower_bound(-p[i][1]); it != sweep.end(); it = sweep.erase(it)) {
                const long long j = it->second;
                const T dx = p[i][0] - p[j][0], dy = p[i][1] - p[j][1];
                if (dy > dx) break;
                g.add_edge(i, j, dx + dy);
            }
            sweep[-p[i][1]] = i;
        }
        // 4 つの向き: (x, y) -> (y, x) -> (-y, x) -> (x, -y) と変える
        for (auto& v : p) {
            if (k % 2 == 0) {
                std::swap(v[0], v[1]);
            } else {
                v[0] = -v[0];
            }
        }
    }
    return minimum_spanning_tree(g);
}
