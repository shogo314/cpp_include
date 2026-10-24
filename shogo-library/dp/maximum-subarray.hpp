#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/dp/maximum-subarray.hpp

/**
 * @file maximum-subarray.hpp
 * @brief 連続部分列の和の最大値 (Kadane のアルゴリズム)
 */

#include <cassert>

/**
 * @brief 空でない連続部分列の和の最大値 O(n)
 * @param v begin()、end() を持つ空でないコンテナ
 */
template <class C>
constexpr typename C::value_type maximum_subarray(const C& v) {
    assert(!v.empty());
    auto itr = v.begin();
    // tmp は、いま見ている要素で終わる連続部分列の和の最大値
    typename C::value_type tmp = *itr++;
    typename C::value_type res = tmp;
    while (itr != v.end()) {
        tmp += *itr;
        if (tmp < *itr) tmp = *itr;
        if (res < tmp) res = tmp;
        ++itr;
    }
    return res;
}

/**
 * @brief 空の部分列 (和 init) も許した連続部分列の和の最大値 O(n)
 * @param v begin()、end() を持つコンテナ
 * @param init 空の部分列の和
 */
template <class C>
constexpr typename C::value_type maximum_subarray(const C& v, typename C::value_type init) {
    typename C::value_type res = init, tmp = init;
    for (const auto& a : v) {
        tmp += a;
        if (tmp < init) tmp = init;
        if (res < tmp) res = tmp;
    }
    return res;
}
