#pragma once
// https://github.com/shogo314/shogo-library/blob/main/src/string/string.hpp

/**
 * @file string.hpp
 * @brief 文字列アルゴリズム (Z-algorithm、KMP、Manacher)
 */

#include <algorithm>
#include <iterator>
#include <vector>

/**
 * @brief Z-algorithm
 * s[0:]とs[i:]のLCPの長さを各iについて求める 全体でO(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<int> z
 */
template <typename It>
std::vector<int> z_algorithm(It begin, It end) {
    int n = end - begin;
    if (n == 0) return {};
    std::vector<int> z(n);
    z[0] = n;
    for (int i = 1, j = 0; i < n;) {
        // [0, j)と[i, i + j)が一致している
        while (i + j < n && begin[j] == begin[i + j]) j++;
        z[i] = j;
        int k = 1;  // [k,)と[i+k,)はLCPが等しいと確定している
        for (; k < j && k + z[k] < j; k++) {
            z[i + k] = z[k];
        }
        i += k;
        if (j) j -= k;
    }
    return z;
}

/**
 * @brief Z-algorithm
 * s[0:]とs[i:]のLCPの長さを各iについて求める 全体でO(N)
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<int> z
 */
template <typename R>
std::vector<int> z_algorithm(const R& s) {
    return z_algorithm(std::begin(s), std::end(s));
}

/**
 * @brief 接頭辞関数 (KMP 法の失敗関数)
 * s[0, i] の接頭辞であり接尾辞でもある部分 (s[0, i] 自身を除く) の最長の長さを各 i について求める 全体で O(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<int> pi
 */
template <typename It>
std::vector<int> prefix_function(It begin, It end) {
    int n = end - begin;
    std::vector<int> pi(n);
    for (int i = 1; i < n; i++) {
        int j = pi[i - 1];
        while (j > 0 && !(begin[i] == begin[j])) j = pi[j - 1];
        if (begin[i] == begin[j]) j++;
        pi[i] = j;
    }
    return pi;
}

/**
 * @brief 接頭辞関数 (KMP 法の失敗関数)
 * s[0, i] の接頭辞であり接尾辞でもある部分 (s[0, i] 自身を除く) の最長の長さを各 i について求める 全体で O(N)
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<int> pi
 */
template <typename R>
std::vector<int> prefix_function(const R& s) {
    return prefix_function(std::begin(s), std::end(s));
}

/**
 * @brief KMP 法で text の中の pattern の出現位置 (開始位置) をすべて昇順に求める O(N + M)
 * pattern が空なら 0, 1, ..., N を返す
 *
 * @tparam R1 配列の型
 * @tparam R2 配列の型
 * @param text 探す対象
 * @param pattern 探す列
 * @return std::vector<int> 出現位置
 */
template <typename R1, typename R2>
std::vector<int> kmp_search(const R1& text, const R2& pattern) {
    auto t = std::begin(text);
    auto p = std::begin(pattern);
    int n = std::end(text) - t, m = std::end(pattern) - p;
    std::vector<int> res;
    if (m == 0) {
        for (int i = 0; i <= n; i++) res.push_back(i);
        return res;
    }
    std::vector<int> pi = prefix_function(pattern);
    for (int i = 0, j = 0; i < n; i++) {
        // pattern[0, j) と text[i - j, i) が一致している
        while (j > 0 && !(t[i] == p[j])) j = pi[j - 1];
        if (t[i] == p[j]) j++;
        if (j == m) {
            res.push_back(i - m + 1);
            j = pi[j - 1];
        }
    }
    return res;
}

/**
 * @brief Manacher 法
 * s[i] を中心とする奇数長の回文の最大の半径 r[i] (s[i - r[i] + 1, i + r[i]) が回文) を各 i について求める 全体で O(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<int> r
 */
template <typename It>
std::vector<int> manacher(It begin, It end) {
    int n = end - begin;
    std::vector<int> r(n);
    // [l, rr] は、これまでに見つけた回文のうち右端が最も右にあるもの
    for (int i = 0, l = 0, rr = -1; i < n; i++) {
        int k = i > rr ? 1 : std::min(r[l + rr - i], rr - i + 1);
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
 * @return std::vector<int> r
 */
template <typename R>
std::vector<int> manacher(const R& s) {
    return manacher(std::begin(s), std::end(s));
}

/**
 * @brief 各位置を中心とする最長の回文の長さ 全体で O(N)
 * 長さ 2N - 1 の配列 len を返す。len[2i] は s[i] を中心とする回文、len[2i + 1] は s[i] と s[i + 1] の間を中心とする回文の最長の長さ
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<int> len
 */
template <typename It>
std::vector<int> palindrome_lengths(It begin, It end) {
    int n = end - begin;
    if (n == 0) return {};
    std::vector<int> odd = manacher(begin, end);
    // even[i] は s[i - 1] と s[i] の間を中心とする偶数長の回文の最大の半径
    std::vector<int> even(n);
    for (int i = 0, l = 0, rr = -1; i < n; i++) {
        int k = i > rr ? 0 : std::min(even[l + rr - i + 1], rr - i + 1);
        while (0 <= i - k - 1 && i + k < n && begin[i - k - 1] == begin[i + k]) k++;
        even[i] = k--;
        if (i + k > rr) {
            l = i - k - 1;
            rr = i + k;
        }
    }
    std::vector<int> len(2 * n - 1);
    for (int i = 0; i < n; i++) len[2 * i] = 2 * odd[i] - 1;
    for (int i = 0; i + 1 < n; i++) len[2 * i + 1] = 2 * even[i + 1];
    return len;
}

/**
 * @brief 各位置を中心とする最長の回文の長さ 全体で O(N)
 * 長さ 2N - 1 の配列 len を返す。len[2i] は s[i] を中心とする回文、len[2i + 1] は s[i] と s[i + 1] の間を中心とする回文の最長の長さ
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<int> len
 */
template <typename R>
std::vector<int> palindrome_lengths(const R& s) {
    return palindrome_lengths(std::begin(s), std::end(s));
}
