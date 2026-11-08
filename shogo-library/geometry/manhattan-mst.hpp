#pragma once

/**
 * @file manhattan-mst.hpp
 * @brief マンハッタン距離の最小全域木
 */

#include <vector>

#include "../graph/graph_util.hpp"
#include "manhattan.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/geometry/manhattan-mst.hpp

/**
 * @brief 点の集合で、2 点を結ぶ辺の重みをマンハッタン距離 |dx| + |dy| とした完全グラフの最小全域木 O(n log n)
 *
 * 各点 i について、i を頂点とする 45 度ずつの 8 つの領域のそれぞれで最も近い点への辺だけを候補にすればよい。
 * 向きを変えながら 4 回掃いて O(n) 本の候補を作り、Kruskal 法にかける
 * @return 最小全域木 (辺の src、dst は点の番号、cost は距離)
 */
template <typename T>
MinimumSpanningTree<T> manhattan_mst(const std::vector<Vector<T, 2>>& p) {
    Graph<T> g(p.size());
    internal_manhattan::octant_nearest(p, [&](long long j, long long i, T d) { g.add_edge(i, j, d); });
    return minimum_spanning_tree(g);
}
