#pragma once
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/string/lyndon.hpp

/**
 * @file lyndon.hpp
 * @brief Lyndon 分解
 */

#include <iterator>
#include <vector>

/**
 * @brief Lyndon 分解 (Duval のアルゴリズム) O(N)
 * s を s = w_1 w_2 ... w_k (各 w_i は Lyndon 文字列で、w_1 >= w_2 >= ... >= w_k) に分ける
 * Lyndon 文字列は、自分のどの真の接尾辞よりも辞書順で真に小さい文字列
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<long long> 区切りの位置 0 = a_0 < a_1 < ... < a_k = N (w_i = s[a_{i-1}, a_i))
 */
template <typename It>
std::vector<long long> lyndon_factorization(It begin, It end) {
    const long long n = end - begin;
    std::vector<long long> res = {0};
    long long i = 0;
    while (i < n) {
        // s[i, j) は、長さ j - k の Lyndon 文字列を何回か繰り返したものと、その接頭辞をつないだもの
        long long j = i + 1, k = i;
        while (j < n && !(begin[j] < begin[k])) {
            k = begin[k] < begin[j] ? i : k + 1;
            j++;
        }
        while (i <= k) {
            i += j - k;
            res.push_back(i);
        }
    }
    return res;
}

/**
 * @brief Lyndon 分解 (Duval のアルゴリズム) O(N)
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<long long> 区切りの位置 0 = a_0 < a_1 < ... < a_k = N
 */
template <typename R>
std::vector<long long> lyndon_factorization(const R& s) {
    return lyndon_factorization(std::begin(s), std::end(s));
}
