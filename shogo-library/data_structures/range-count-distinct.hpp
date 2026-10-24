#pragma once

/**
 * @file range-count-distinct.hpp
 * @brief 区間内の異なる値の個数
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "wavelet-matrix.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/range-count-distinct.hpp

/**
 * @brief 固定した列について、区間内の異なる値の個数をオンラインで答える
 *
 * 位置 i の前に同じ値が現れた最後の位置を prev[i] (なければ -1) とすると、[l, r) の異なる値の個数は
 * l <= i < r かつ prev[i] < l となる i の個数なので、prev を Wavelet Matrix に載せて数える
 * 構築 O(n log n)、クエリ O(log n)
 * 例:
 * RangeCountDistinct rcd(a);
 * long long k = rcd.count(l, r);
 *
 * @tparam T 値の型 (operator< で比べられること)
 */
template <typename T>
class RangeCountDistinct {
    long long n;
    WaveletMatrix<long long> wm;  // prev[i] + 1 を載せる

    static std::vector<long long> build_prev(const std::vector<T>& a) {
        const long long n = a.size();
        std::vector<std::pair<T, long long>> sorted(n);
        for (long long i = 0; i < n; i++) sorted[i] = {a[i], i};
        std::sort(sorted.begin(), sorted.end());
        std::vector<long long> prev(n, 0);
        for (long long i = 1; i < n; i++) {
            // 値が同じ (前のほうが小さくない) なら、前の位置が直前の出現
            if (!(sorted[i - 1].first < sorted[i].first)) prev[sorted[i].second] = sorted[i - 1].second + 1;
        }
        return prev;
    }

   public:
    /**
     * @brief 列 a から作る
     */
    explicit RangeCountDistinct(const std::vector<T>& a) : n(a.size()), wm(build_prev(a)) {}
    /**
     * @brief 要素数
     */
    long long size() const { return n; }
    /**
     * @brief a[l], ..., a[r-1] に現れる異なる値の個数
     */
    long long count(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        return wm.count(l, r, 0, l + 1);
    }
};

template <typename T>
RangeCountDistinct(std::vector<T>) -> RangeCountDistinct<T>;
