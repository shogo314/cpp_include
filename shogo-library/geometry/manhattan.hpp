#pragma once

/**
 * @file manhattan.hpp
 * @brief マンハッタン距離 (最近点対・最遠点対、各点から最も近い点・最も遠い点)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <numeric>
#include <vector>

#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/geometry/manhattan.hpp

namespace internal_manhattan {
/**
 * @brief 各点 j と、j を頂点とする 45 度ずつの 8 つの領域のうち 4 つのそれぞれについて、その領域で j に最も近い点 i を f(j, i, 距離) で渡す O(n log n)
 * 残りの 4 つの領域は、点を原点について反転させて呼べば得られる
 */
template <typename T, typename F>
void octant_nearest(std::vector<Vector<T, 2>> p, F f) {
    const long long n = p.size();
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
                f(j, i, dx + dy);
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
}
}  // namespace internal_manhattan

/**
 * @brief マンハッタン距離の最近点対・最遠点対の戻り値
 */
template <typename T>
struct ManhattanPair {
    T dist;          //!< 距離
    long long i, j;  //!< 2 点の添字 (i != j)
};

/**
 * @brief 各点から、マンハッタン距離で最も近いほかの点の添字 O(n log n)
 * 8 つの領域のそれぞれで最も近い点のうち、最も近いもの。点が 1 個なら -1
 */
template <typename T>
std::vector<long long> manhattan_nearest_points(const std::vector<Vector<T, 2>>& p) {
    const long long n = p.size();
    std::vector<long long> res(n, -1);
    std::vector<T> best(n);
    // 同じ位置の点どうしは、どの向きで掃いても並ぶ順が変わらず片方からしか見つからないので、先に組にする
    std::vector<long long> id(n);
    std::iota(id.begin(), id.end(), 0);
    std::sort(id.begin(), id.end(), [&](long long i, long long j) { return p[i] < p[j]; });
    for (long long k = 0; k + 1 < n; k++) {
        if (p[id[k]] == p[id[k + 1]]) res[id[k]] = id[k + 1], res[id[k + 1]] = id[k], best[id[k]] = best[id[k + 1]] = 0;
    }
    auto update = [&](long long j, long long i, T d) {
        if (res[j] == -1 || d < best[j]) res[j] = i, best[j] = d;
    };
    internal_manhattan::octant_nearest(p, update);
    std::vector<Vector<T, 2>> q(p);
    for (auto& v : q) v = -v;
    internal_manhattan::octant_nearest(q, update);
    return res;
}

/**
 * @brief 各点から、マンハッタン距離で最も遠い点の添字 O(n)
 * |dx| + |dy| = max(|du|, |dv|) (u = x + y, v = x - y) なので、u と v が最大・最小の点のどれか。点が 1 個なら -1
 */
template <typename T>
std::vector<long long> manhattan_farthest_points(const std::vector<Vector<T, 2>>& p) {
    const long long n = p.size();
    if (n <= 1) return std::vector<long long>(n, -1);
    // ext[0], ext[1]: u が最小・最大の点、ext[2], ext[3]: v が最小・最大の点
    auto u = [&](long long i) { return p[i][0] + p[i][1]; };
    auto v = [&](long long i) { return p[i][0] - p[i][1]; };
    long long ext[4] = {0, 0, 0, 0};
    for (long long i = 1; i < n; i++) {
        if (u(i) < u(ext[0])) ext[0] = i;
        if (u(i) > u(ext[1])) ext[1] = i;
        if (v(i) < v(ext[2])) ext[2] = i;
        if (v(i) > v(ext[3])) ext[3] = i;
    }
    std::vector<long long> res(n);
    for (long long i = 0; i < n; i++) {
        long long b = -1;
        T best = 0;
        for (long long j : ext) {
            const T d = std::abs(p[i][0] - p[j][0]) + std::abs(p[i][1] - p[j][1]);
            if (j != i && (b == -1 || d > best)) b = j, best = d;
        }
        // 4 点がすべて i 自身なら、すべての点が同じ位置にある
        res[i] = b == -1 ? (i == 0 ? 1 : 0) : b;
    }
    return res;
}

/**
 * @brief マンハッタン距離の最近点対 (点は 2 個以上) O(n log n)
 * 最小全域木の辺の候補 (各点から 4 つの領域で最も近い点への辺) に、最も近い 2 点を結ぶ辺が含まれる
 */
template <typename T>
ManhattanPair<T> manhattan_closest_pair(const std::vector<Vector<T, 2>>& p) {
    assert(p.size() >= 2 && "manhattan_closest_pair: fewer than 2 points");
    ManhattanPair<T> res{T(0), -1, -1};
    internal_manhattan::octant_nearest(p, [&](long long j, long long i, T d) {
        if (res.i == -1 || d < res.dist) res = {d, std::min(i, j), std::max(i, j)};
    });
    return res;
}

/**
 * @brief マンハッタン距離の最遠点対 (点は 2 個以上) O(n)
 * u = x + y, v = x - y の最大と最小の差のうち大きい方
 */
template <typename T>
ManhattanPair<T> manhattan_farthest_pair(const std::vector<Vector<T, 2>>& p) {
    const long long n = p.size();
    assert(n >= 2 && "manhattan_farthest_pair: fewer than 2 points");
    const std::vector<long long> f = manhattan_farthest_points(p);
    ManhattanPair<T> res{T(0), -1, -1};
    for (long long i = 0; i < n; i++) {
        const long long j = f[i];
        const T d = std::abs(p[i][0] - p[j][0]) + std::abs(p[i][1] - p[j][1]);
        if (res.i == -1 || d > res.dist) res = {d, std::min(i, j), std::max(i, j)};
    }
    return res;
}
