#pragma once
// https://github.com/shogo314/shogo-library/blob/cfbc779abd4bff3efa5b12de9b69f0aabc01392d/src/others/inversion-number.hpp

/**
 * @file inversion-number.hpp
 * @brief 転倒数
 */

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

/**
 * @brief 転倒数 (i < j かつ comp(a[j], a[i]) となる組 (i, j) の個数) を求める O(n log n)
 *
 * マージソートで数えるので、座標圧縮は不要。再帰を使わない
 * @param comp 大小の比較。std::greater<T>() を渡すと、i < j かつ a[i] < a[j] となる組の個数になる
 */
template <typename T, typename Compare = std::less<T>>
long long inversion_number(std::vector<T> a, Compare comp = Compare()) {
    int n = a.size();
    std::vector<T> buf(n);
    long long res = 0;
    for (int w = 1; w < n; w *= 2) {
        // 長さ w のソート済みの区間を 2 つずつマージする
        for (int l = 0; l < n; l += 2 * w) {
            int m = std::min(l + w, n), r = std::min(l + 2 * w, n);
            int i = l, j = m, k = l;
            while (i < m && j < r) {
                if (comp(a[j], a[i])) {
                    // a[j] は左側の残り a[i, m) のすべてより小さい
                    res += m - i;
                    buf[k++] = std::move(a[j++]);
                } else {
                    buf[k++] = std::move(a[i++]);
                }
            }
            while (i < m) buf[k++] = std::move(a[i++]);
            while (j < r) buf[k++] = std::move(a[j++]);
        }
        std::swap(a, buf);
    }
    return res;
}
