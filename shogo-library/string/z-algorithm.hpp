#pragma once
// https://github.com/shogo314/shogo-library/blob/77ebb090933789a5c7f4dc5561b48ac9861e1e0b/src/string/z-algorithm.hpp

/**
 * @file z-algorithm.hpp
 * @brief Z-algorithm
 */

#include <iterator>
#include <vector>

/**
 * @brief Z-algorithm
 * s[0:]とs[i:]のLCPの長さを各iについて求める 全体でO(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<long long> z
 */
template <typename It>
std::vector<long long> z_algorithm(It begin, It end) {
    long long n = end - begin;
    if (n == 0) return {};
    std::vector<long long> z(n);
    z[0] = n;
    for (long long i = 1, j = 0; i < n;) {
        // [0, j)と[i, i + j)が一致している
        while (i + j < n && begin[j] == begin[i + j]) j++;
        z[i] = j;
        long long k = 1;  // [k,)と[i+k,)はLCPが等しいと確定している
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
 * @return std::vector<long long> z
 */
template <typename R>
std::vector<long long> z_algorithm(const R& s) {
    return z_algorithm(std::begin(s), std::end(s));
}
