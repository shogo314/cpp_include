#pragma once

/**
 * @file subarray-sum.hpp
 * @brief 和が条件を満たす区間の個数
 */

#include <algorithm>
#include <type_traits>
#include <vector>

#include "../utility/two-pointers.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/others/subarray-sum.hpp

namespace internal_subarray_sum {
// i < j かつ p[j] - p[i] <= k となる組 (i, j) の個数
// p が広義単調増加なら尺取り法で O(n)、そうでなければマージソートで O(n log n) (再帰を使わない)
template <typename T>
long long count_pairs_at_most(std::vector<T> p, T k) {
    const int n = p.size();
    long long res = 0;
    if (std::is_sorted(p.begin(), p.end())) {
        // i ごとに、p[j] - p[i] <= k となる j は [i, r[i]) (p が単調なので、i を増やしても条件は崩れない)
        const auto r = two_pointers(n, [&](long long i, long long j) { return p[j] - p[i] <= k; });
        for (int i = 0; i < n; i++) res += std::max(r[i] - i - 1, 0LL);
        return res;
    }
    std::vector<T> buf(n);
    for (int w = 1; w < n; w *= 2) {
        for (int l = 0; l < n; l += 2 * w) {
            const int m = std::min(l + w, n), r = std::min(l + 2 * w, n);
            // 左の [l, m) と右の [m, r) はそれぞれ昇順。右の各 p[j] について、左で p[i] >= p[j] - k となるものを数える
            for (int i = l, j = m; j < r; j++) {
                while (i < m && p[i] < p[j] - k) i++;
                res += m - i;
            }
            std::merge(p.begin() + l, p.begin() + m, p.begin() + m, p.begin() + r, buf.begin() + l);
            std::copy(buf.begin() + l, buf.begin() + r, p.begin() + l);
        }
    }
    return res;
}
// 先頭に 0 を付けた累積和
template <typename T>
std::vector<T> prefix_sums(const std::vector<T>& a) {
    std::vector<T> p(a.size() + 1, T{});
    for (size_t i = 0; i < a.size(); i++) p[i + 1] = p[i] + a[i];
    return p;
}
}  // namespace internal_subarray_sum

/**
 * @brief 和が k 以下の区間 [l, r) (l < r) の個数
 * 累積和 P について、l < r かつ P[r] - P[l] <= k となる組を数える。要素がすべて 0 以上なら尺取り法で O(n)、そうでなければ O(n log n)
 */
template <typename T>
long long count_subarrays_with_sum_at_most(const std::vector<T>& a, T k) {
    return internal_subarray_sum::count_pairs_at_most(internal_subarray_sum::prefix_sums(a), k);
}

/**
 * @brief 和がちょうど k の区間 [l, r) (l < r) の個数
 * (和が k 以下の個数) - (和が k - 1 以下の個数) で求める。要素がすべて 0 以上なら O(n)、そうでなければ O(n log n)
 * @note 要素は整数型であること
 */
template <typename T>
long long count_subarrays_with_sum(const std::vector<T>& a, T k) {
    static_assert(std::is_integral_v<T>, "count_subarrays_with_sum: T must be an integer type");
    const auto p = internal_subarray_sum::prefix_sums(a);
    return internal_subarray_sum::count_pairs_at_most(p, k) - internal_subarray_sum::count_pairs_at_most(p, k - 1);
}
