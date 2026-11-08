#pragma once
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/string/kmp.hpp

/**
 * @file kmp.hpp
 * @brief 接頭辞関数と KMP 法
 */

#include <iterator>
#include <vector>

/**
 * @brief 接頭辞関数 (KMP 法の失敗関数)
 * s[0, i] の接頭辞であり接尾辞でもある部分 (s[0, i] 自身を除く) の最長の長さを各 i について求める 全体で O(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<long long> pi
 */
template <typename It>
std::vector<long long> prefix_function(It begin, It end) {
    long long n = end - begin;
    std::vector<long long> pi(n);
    for (long long i = 1; i < n; i++) {
        long long j = pi[i - 1];
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
 * @return std::vector<long long> pi
 */
template <typename R>
std::vector<long long> prefix_function(const R& s) {
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
 * @return std::vector<long long> 出現位置
 */
template <typename R1, typename R2>
std::vector<long long> kmp_search(const R1& text, const R2& pattern) {
    auto t = std::begin(text);
    auto p = std::begin(pattern);
    long long n = std::end(text) - t, m = std::end(pattern) - p;
    std::vector<long long> res;
    if (m == 0) {
        for (long long i = 0; i <= n; i++) res.push_back(i);
        return res;
    }
    std::vector<long long> pi = prefix_function(pattern);
    for (long long i = 0, j = 0; i < n; i++) {
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
