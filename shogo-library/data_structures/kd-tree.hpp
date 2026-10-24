#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/kd-tree.hpp

/**
 * @file kd-tree.hpp
 * @brief kd-tree
 */

#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>

/**
 * @brief 2 次元の点の集合 (静的) について、長方形領域に入る点を列挙する kd-tree
 *
 * x と y で交互に中央値で分け、各部分木の点を囲む長方形を持つ。構築 O(n log n)、
 * 列挙は O(√n + 列挙する個数)、個数を数えるのも O(√n)
 * 例:
 * KDTree<long long> kd(points);  // points[i] = {x, y}
 * auto ids = kd.enumerate(xl, xr, yl, yr);  // xl <= x < xr かつ yl <= y < yr の点の添字
 *
 * @tparam T 座標の型
 */
template <typename T>
class KDTree {
    long long n;
    std::vector<long long> ord;  // 木の順に並べた点の添字
    std::vector<T> x, y;         // 木の順に並べた座標
    // [l, r) の部分木の点を囲む長方形を、中央 m = (l + r) / 2 の位置に持つ
    std::vector<T> min_x, max_x, min_y, max_y;

    void build(long long l, long long r, bool by_x, const std::vector<std::pair<T, T>>& points) {
        if (l >= r) return;
        long long m = (l + r) / 2;
        std::nth_element(ord.begin() + l, ord.begin() + m, ord.begin() + r, [&](long long i, long long j) {
            return by_x ? points[i].first < points[j].first : points[i].second < points[j].second;
        });
        build(l, m, !by_x, points);
        build(m + 1, r, !by_x, points);
        min_x[m] = max_x[m] = points[ord[m]].first;
        min_y[m] = max_y[m] = points[ord[m]].second;
        // 子の部分木が空でなければ、その長方形を合わせる
        for (long long c : {l < m ? (l + m) / 2 : -1, m + 1 < r ? (m + 1 + r) / 2 : -1}) {
            if (c == -1) continue;
            min_x[m] = std::min(min_x[m], min_x[c]), max_x[m] = std::max(max_x[m], max_x[c]);
            min_y[m] = std::min(min_y[m], min_y[c]), max_y[m] = std::max(max_y[m], max_y[c]);
        }
    }
    // 長方形に入る点を、1 点ずつ point(i) で、部分木ごと入るなら range(l, r) で報告する (i, l, r は木の順の位置)
    template <typename F, typename G>
    void query(long long l, long long r, T xl, T xr, T yl, T yr, const F& point, const G& range) const {
        if (l >= r) return;
        long long m = (l + r) / 2;
        if (max_x[m] < xl || xr <= min_x[m] || max_y[m] < yl || yr <= min_y[m]) return;
        if (xl <= min_x[m] && max_x[m] < xr && yl <= min_y[m] && max_y[m] < yr) {
            range(l, r);
            return;
        }
        if (xl <= x[m] && x[m] < xr && yl <= y[m] && y[m] < yr) point(m);
        query(l, m, xl, xr, yl, yr, point, range);
        query(m + 1, r, xl, xr, yl, yr, point, range);
    }

   public:
    /**
     * @brief 点 i の座標が points[i] = {x, y} の kd-tree を作る
     */
    explicit KDTree(const std::vector<std::pair<T, T>>& points)
        : n(points.size()), ord(n), x(n), y(n), min_x(n), max_x(n), min_y(n), max_y(n) {
        std::iota(ord.begin(), ord.end(), 0);
        build(0, n, true, points);
        for (long long i = 0; i < n; i++) x[i] = points[ord[i]].first, y[i] = points[ord[i]].second;
    }
    /**
     * @brief 点の個数
     */
    long long size() const { return n; }
    /**
     * @brief xl <= x < xr かつ yl <= y < yr の点の添字 (順不同)
     */
    std::vector<long long> enumerate(T xl, T xr, T yl, T yr) const {
        std::vector<long long> res;
        query(0, n, xl, xr, yl, yr, [&](long long i) { res.push_back(ord[i]); }, [&](long long l, long long r) { res.insert(res.end(), ord.begin() + l, ord.begin() + r); });
        return res;
    }
    /**
     * @brief xl <= x < xr かつ yl <= y < yr の点の個数
     */
    long long count(T xl, T xr, T yl, T yr) const {
        long long res = 0;
        query(0, n, xl, xr, yl, yr, [&](long long) { res++; }, [&](long long l, long long r) { res += r - l; });
        return res;
    }
};
