#pragma once
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/dp/longest-increasing-subsequence.hpp

/**
 * @file longest-increasing-subsequence.hpp
 * @brief 最長増加部分列 (LIS)
 */

#include <algorithm>
#include <functional>
#include <vector>

/**
 * @brief a[i] で終わる最長増加部分列の長さを、各 i について求める O(n log n)
 * @param strict true なら狭義単調増加 (comp(前, 後) が成り立つ)、false なら広義単調増加 (comp(後, 前) が成り立たない)
 * @param comp 大小の比較。std::greater<T>() を渡すと減少列になる
 */
template <typename T, typename Compare = std::less<T>>
std::vector<long long> longest_increasing_subsequence_lengths(const std::vector<T>& a, bool strict = true,
                                                              Compare comp = Compare()) {
    // tail[k] は、長さ k + 1 の増加部分列の末尾として最も小さい値
    std::vector<T> tail;
    std::vector<long long> res(a.size());
    for (long long i = 0; i < static_cast<long long>(a.size()); i++) {
        auto it = strict ? std::lower_bound(tail.begin(), tail.end(), a[i], comp)
                         : std::upper_bound(tail.begin(), tail.end(), a[i], comp);
        res[i] = it - tail.begin() + 1;
        if (it == tail.end())
            tail.push_back(a[i]);
        else
            *it = a[i];
    }
    return res;
}

/**
 * @brief 最長増加部分列の長さ O(n log n)
 * @param strict true なら狭義単調増加 (comp(前, 後) が成り立つ)、false なら広義単調増加 (comp(後, 前) が成り立たない)
 * @param comp 大小の比較。std::greater<T>() を渡すと減少列になる
 */
template <typename T, typename Compare = std::less<T>>
long long longest_increasing_subsequence(const std::vector<T>& a, bool strict = true, Compare comp = Compare()) {
    // tail[k] は、長さ k + 1 の増加部分列の末尾として最も小さい値
    std::vector<T> tail;
    for (const T& x : a) {
        auto it = strict ? std::lower_bound(tail.begin(), tail.end(), x, comp)
                         : std::upper_bound(tail.begin(), tail.end(), x, comp);
        if (it == tail.end())
            tail.push_back(x);
        else
            *it = x;
    }
    return tail.size();
}

/**
 * @brief 最長増加部分列を 1 つ求め、その添字の昇順のリストを返す O(n log n)
 * @param strict true なら狭義単調増加 (comp(前, 後) が成り立つ)、false なら広義単調増加 (comp(後, 前) が成り立たない)
 * @param comp 大小の比較。std::greater<T>() を渡すと減少列になる
 */
template <typename T, typename Compare = std::less<T>>
std::vector<long long> longest_increasing_subsequence_restore(const std::vector<T>& a, bool strict = true,
                                                              Compare comp = Compare()) {
    std::vector<long long> len = longest_increasing_subsequence_lengths(a, strict, comp);
    long long k = len.empty() ? 0 : *std::max_element(len.begin(), len.end());
    // 後ろから、長さが 1 ずつ減り、値が条件を満たす添字を選ぶ
    std::vector<long long> res(k);
    long long last = -1;
    for (long long i = static_cast<long long>(a.size()) - 1; i >= 0 && k > 0; i--) {
        if (len[i] != k) continue;
        if (last != -1 && !(strict ? comp(a[i], a[last]) : !comp(a[last], a[i]))) continue;
        res[--k] = i;
        last = i;
    }
    return res;
}
