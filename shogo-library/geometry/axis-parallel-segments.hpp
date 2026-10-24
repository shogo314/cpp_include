#pragma once

/**
 * @file axis-parallel-segments.hpp
 * @brief 軸に平行な線分の交点の個数
 */

#include <algorithm>
#include <cassert>
#include <tuple>
#include <vector>

#include "../algebra/group.hpp"
#include "../data_structures/fenwick-tree.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/geometry/axis-parallel-segments.hpp

/**
 * @brief 水平か垂直な線分だけがあるとき、水平な線分と垂直な線分が共有点を持つ組の個数 (走査線) O(n log n)
 *
 * y の小さい順に走査し、y 方向に今またいでいる垂直な線分の x を Fenwick 木で数える
 * 端点で接するものも数える。平行な線分どうしの共有点は数えない。長さ 0 の線分は垂直な線分として扱う
 * @param segments 線分の端点の組 (各線分は x 座標か y 座標が等しいこと)
 * @return long long 共有点を持つ (水平な線分, 垂直な線分) の組の個数
 */
template <typename T>
long long count_axis_parallel_intersections(const std::vector<std::pair<Vector<T, 2>, Vector<T, 2>>>& segments) {
    std::vector<T> xs;
    // (y, 種類, x1, x2)。種類は 0: 垂直な線分の始まり、1: 水平な線分、2: 垂直な線分の終わり (同じ y ではこの順に処理する)
    std::vector<std::tuple<T, int, T, T>> events;
    for (const auto& [p, q] : segments) {
        if (p[0] == q[0]) {
            T y1 = std::min(p[1], q[1]), y2 = std::max(p[1], q[1]);
            events.emplace_back(y1, 0, p[0], p[0]);
            events.emplace_back(y2, 2, p[0], p[0]);
            xs.push_back(p[0]);
        } else {
            assert(p[1] == q[1] && "count_axis_parallel_intersections: segment is not axis-parallel");
            events.emplace_back(p[1], 1, std::min(p[0], q[0]), std::max(p[0], q[0]));
        }
    }
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    auto index = [&](T x) { return std::lower_bound(xs.begin(), xs.end(), x) - xs.begin(); };
    std::sort(events.begin(), events.end());
    FenwickTree<group::Sum<long long>> ft(xs.size());
    long long res = 0;
    for (const auto& [y, type, x1, x2] : events) {
        if (type == 0) {
            ft.apply(index(x1), 1);
        } else if (type == 2) {
            ft.apply(index(x1), -1);
        } else {
            // x1 <= x <= x2 にある垂直な線分の個数
            res += ft.prod(index(x1), std::upper_bound(xs.begin(), xs.end(), x2) - xs.begin());
        }
    }
    return res;
}
