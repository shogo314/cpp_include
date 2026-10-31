#pragma once

/**
 * @file point-pair.hpp
 * @brief 最近点対・最遠点対
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <stack>
#include <utility>
#include <vector>

#include "polygon.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/460c030aa07ed32c4e107d6268544d802c0bc521/src/geometry/point-pair.hpp

/**
 * @brief closest_point_pair_squared の戻り値
 */
template <typename T>
struct ClosestPairSquared {
    T squared_dist;  //!< 最近点対の距離の2乗
    long long i, j;  //!< 最近点対の添字
};
/**
 * @brief closest_point_pair の戻り値
 */
struct ClosestPair {
    double dist;     //!< 最近点対の距離
    long long i, j;  //!< 最近点対の添字
};

/**
 * @brief 最近点対
 *
 * @tparam T 座標の型
 * @param points 頂点リスト (2個以上)
 * @return ClosestPairSquared<T> 最近点対の距離の2乗と添字
 */
template <typename T>
ClosestPairSquared<T> closest_point_pair_squared(const std::vector<Vector<T, 2>>& points) {
    assert(points.size() >= 2 && "closest_point_pair_squared: fewer than 2 points");
    std::vector<int> sorted_idx(points.size());
    std::iota(sorted_idx.begin(), sorted_idx.end(), 0);
    std::sort(sorted_idx.begin(), sorted_idx.end(), [&](int i, int j) {
        return points[i][0] < points[j][0];
    });
    std::vector<int> left(1, 0);                                        // 左端
    std::vector<int> right(1, points.size());                           // 右端
    std::vector<int> mid(1, points.size() / 2);                         // 中央
    std::vector<int> par(1, -1);                                        // 親
    std::vector<T> min_dist_squared(1, std::numeric_limits<T>::max());  // 最小距離の2乗
    std::vector<std::pair<int, int>> min_dist_pair(1, {-1, -1});        // 最小距離の点対
    std::stack<int> stk;
    stk.push(~0);
    stk.push(0);
    while (!stk.empty()) {
        int u = stk.top();
        stk.pop();
        if (u >= 0) {
            // 行きがけ
            if (left[u] + 1 < mid[u]) {
                int v = left.size();
                left.push_back(left[u]);
                right.push_back(mid[u]);
                mid.push_back((left[u] + mid[u]) / 2);
                par.push_back(u);
                min_dist_squared.push_back(std::numeric_limits<T>::max());
                min_dist_pair.push_back({-1, -1});
                stk.push(~v);
                stk.push(v);
            }
            if (mid[u] + 1 < right[u]) {
                int v = left.size();
                left.push_back(mid[u]);
                right.push_back(right[u]);
                mid.push_back((mid[u] + right[u]) / 2);
                par.push_back(u);
                min_dist_squared.push_back(std::numeric_limits<T>::max());
                min_dist_pair.push_back({-1, -1});
                stk.push(~v);
                stk.push(v);
            }
        } else {
            // 帰りがけ
            u = ~u;
            std::vector<int> cand;
            for (int i = left[u]; i < right[u]; i++) {
                T dx = points[sorted_idx[i]][0] - points[sorted_idx[mid[u]]][0];
                if (dx * dx < min_dist_squared[u]) cand.push_back(i);
            }
            std::sort(cand.begin(), cand.end(), [&](int i, int j) {
                return points[sorted_idx[i]][1] < points[sorted_idx[j]][1];
            });
            for (int i = 0; i < static_cast<int>(cand.size()); i++) {
                for (int j = i + 1; j < static_cast<int>(cand.size()); j++) {
                    T dx = points[sorted_idx[cand[j]]][0] - points[sorted_idx[cand[i]]][0];
                    T dy = points[sorted_idx[cand[j]]][1] - points[sorted_idx[cand[i]]][1];
                    if (dy * dy >= min_dist_squared[u]) break;
                    T d = dx * dx + dy * dy;
                    if (d < min_dist_squared[u]) {
                        min_dist_squared[u] = d;
                        min_dist_pair[u] = {sorted_idx[cand[i]], sorted_idx[cand[j]]};
                    }
                }
            }
            if (par[u] != -1 && min_dist_squared[u] < min_dist_squared[par[u]]) {
                min_dist_squared[par[u]] = min_dist_squared[u];
                min_dist_pair[par[u]] = min_dist_pair[u];
            }
        }
    }
    return {min_dist_squared[0], min_dist_pair[0].first, min_dist_pair[0].second};
}

/**
 * @brief 最近点対
 *
 * @tparam T 座標の型
 * @param points 頂点リスト (2個以上)
 * @return ClosestPair 最近点対の距離と添字
 */
template <typename T>
ClosestPair closest_point_pair(const std::vector<Vector<T, 2>>& points) {
    auto [d2, i, j] = closest_point_pair_squared(points);
    return {std::sqrt(static_cast<double>(d2)), i, j};
}

/**
 * @brief farthest_point_pair_squared の戻り値
 */
template <typename T>
struct FarthestPairSquared {
    T squared_dist;  //!< 最遠点対の距離の2乗
    long long i, j;  //!< 最遠点対の添字
};
/**
 * @brief farthest_point_pair の戻り値
 */
struct FarthestPair {
    double dist;     //!< 最遠点対の距離
    long long i, j;  //!< 最遠点対の添字
};

/**
 * @brief 最遠点対 (凸包を作り、回転キャリパー法で対蹠点の組を調べる) O(n log n)
 *
 * 整数型でも使える。座標の差の2乗の和の2倍が T に収まる必要がある
 * @tparam T 座標の型
 * @param points 頂点リスト (2個以上)
 * @return FarthestPairSquared<T> 最遠点対の距離の2乗と添字 (i != j)
 */
template <typename T>
FarthestPairSquared<T> farthest_point_pair_squared(const std::vector<Vector<T, 2>>& points) {
    const int n = static_cast<int>(points.size());
    assert(n >= 2);
    // 凸包の頂点を、もとの添字に戻せるようにしておく
    std::vector<std::pair<Vector<T, 2>, int>> sorted(n);
    for (int i = 0; i < n; i++) sorted[i] = {points[i], i};
    std::sort(sorted.begin(), sorted.end());
    auto index_of = [&](const Vector<T, 2>& p) {
        return std::lower_bound(sorted.begin(), sorted.end(), std::make_pair(p, -1))->second;
    };
    std::vector<Vector<T, 2>> hull = convex_hull(points);
    const int k = static_cast<int>(hull.size());
    if (k == 1) return {T(0), sorted[0].second, sorted[1].second};  // すべて同じ点
    FarthestPairSquared<T> res{T(0), -1, -1};
    auto update = [&](int a, int b) {
        T d = (hull[a] - hull[b]).squared_norm();
        if (res.i == -1 || d > res.squared_dist) res = {d, a, b};
    };
    if (k == 2) {
        update(0, 1);
    } else {
        // 辺 i→i+1 に対して、最も遠い頂点 j を反時計回りに進めていく
        int j = 1;
        for (int i = 0; i < k; i++) {
            int ni = i + 1 == k ? 0 : i + 1;
            while (true) {
                int nj = j + 1 == k ? 0 : j + 1;
                if (cross(hull[ni] - hull[i], hull[nj] - hull[j]) > 0)
                    j = nj;
                else
                    break;
            }
            update(i, j);
            update(ni, j);
        }
    }
    return {res.squared_dist, index_of(hull[res.i]), index_of(hull[res.j])};
}

/**
 * @brief 最遠点対
 *
 * @tparam T 座標の型
 * @param points 頂点リスト (2個以上)
 * @return FarthestPair 最遠点対の距離と添字
 */
template <typename T>
FarthestPair farthest_point_pair(const std::vector<Vector<T, 2>>& points) {
    auto [d2, i, j] = farthest_point_pair_squared(points);
    return {std::sqrt(static_cast<double>(d2)), i, j};
}
/**
 * @deprecated closest_point_pair_squared を使うこと(綴りの誤りを直して改名した)
 */
template <typename T>
[[deprecated("use closest_point_pair_squared")]] ClosestPairSquared<T> closest_point_pair_squred(const std::vector<Vector<T, 2>>& points) {
    return closest_point_pair_squared(points);
}
