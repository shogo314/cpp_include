#pragma once
// https://github.com/shogo314/shogo-library/blob/adb06bae4b0e202b3a1fe53ef9f9a1b30077d8cc/src/string/manacher.hpp

/**
 * @file manacher.hpp
 * @brief Manacher 法と各位置を中心とする最長の回文
 */

#include <algorithm>
#include <iterator>
#include <vector>

/**
 * @brief Manacher 法
 * s[i] を中心とする奇数長の回文の最大の半径 r[i] (s[i - r[i] + 1, i + r[i]) が回文) を各 i について求める 全体で O(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<long long> r
 */
template <typename It>
std::vector<long long> manacher(It begin, It end) {
    long long n = end - begin;
    std::vector<long long> r(n);
    // [l, rr] は、これまでに見つけた回文のうち右端が最も右にあるもの
    for (long long i = 0, l = 0, rr = -1; i < n; i++) {
        long long k = i > rr ? 1 : std::min(r[l + rr - i], rr - i + 1);
        while (0 <= i - k && i + k < n && begin[i - k] == begin[i + k]) k++;
        r[i] = k--;
        if (i + k > rr) {
            l = i - k;
            rr = i + k;
        }
    }
    return r;
}

/**
 * @brief Manacher 法
 * s[i] を中心とする奇数長の回文の最大の半径 r[i] (s[i - r[i] + 1, i + r[i]) が回文) を各 i について求める 全体で O(N)
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<long long> r
 */
template <typename R>
std::vector<long long> manacher(const R& s) {
    return manacher(std::begin(s), std::end(s));
}

/**
 * @brief 各位置を中心とする最長の回文の長さ 全体で O(N)
 * 長さ 2N - 1 の配列 len を返す。len[2i] は s[i] を中心とする回文、len[2i + 1] は s[i] と s[i + 1] の間を中心とする回文の最長の長さ
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<long long> len
 */
template <typename It>
std::vector<long long> palindrome_lengths(It begin, It end) {
    long long n = end - begin;
    if (n == 0) return {};
    std::vector<long long> odd = manacher(begin, end);
    // even[i] は s[i - 1] と s[i] の間を中心とする偶数長の回文の最大の半径
    std::vector<long long> even(n);
    for (long long i = 0, l = 0, rr = -1; i < n; i++) {
        long long k = i > rr ? 0 : std::min(even[l + rr - i + 1], rr - i + 1);
        while (0 <= i - k - 1 && i + k < n && begin[i - k - 1] == begin[i + k]) k++;
        even[i] = k--;
        if (i + k > rr) {
            l = i - k - 1;
            rr = i + k;
        }
    }
    std::vector<long long> len(2 * n - 1);
    for (long long i = 0; i < n; i++) len[2 * i] = 2 * odd[i] - 1;
    for (long long i = 0; i + 1 < n; i++) len[2 * i + 1] = 2 * even[i + 1];
    return len;
}

/**
 * @brief 各位置を中心とする最長の回文の長さ 全体で O(N)
 * 長さ 2N - 1 の配列 len を返す。len[2i] は s[i] を中心とする回文、len[2i + 1] は s[i] と s[i + 1] の間を中心とする回文の最長の長さ
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<long long> len
 */
template <typename R>
std::vector<long long> palindrome_lengths(const R& s) {
    return palindrome_lengths(std::begin(s), std::end(s));
}
