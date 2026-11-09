#pragma once

/**
 * @file binary-search.hpp
 * @brief 二分探索
 */

#include <cmath>
#include <type_traits>

#include "../base/functions.hpp"
#include "traits.hpp"
// https://github.com/shogo314/shogo-library/blob/27998142f20ef4e094add3b53cc9c070db4ddbf8/src/utility/binary-search.hpp

/**
 * f(ok) が真、f(ng) が偽として、境界で f が真になる側の値を返す
 * 整数なら ok と ng の差が 1 になるまで、浮動小数点数なら 100 回二分する
 */
template <typename M, typename N, class F, ENABLE_IF_T(std::is_integral_v<std::common_type_t<M, N>>and std::is_invocable_r_v<bool, F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(const M& ok, const N& ng, F f) {
    using T = std::common_type_t<M, N>;
    using U = std::make_unsigned_t<T>;
    T _ok = ok, _ng = ng;
    // 差は符号なしで求めるとあふれない (LLONG_MIN と LLONG_MAX の差も U に収まる)
    // 中点は midpoint で求める (あふれず、ok の側に丸める)
    while ((_ok < _ng ? static_cast<U>(static_cast<U>(_ng) - static_cast<U>(_ok)) : static_cast<U>(static_cast<U>(_ok) - static_cast<U>(_ng))) > 1) {
        T mid = midpoint(_ok, _ng);
        if (f(mid)) {
            _ok = mid;
        } else {
            _ng = mid;
        }
    }
    return _ok;
}

template <typename M, typename N, class F, ENABLE_IF_T(!std::is_integral_v<std::common_type_t<M, N>> && std::is_invocable_r_v<bool, F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(const M& ok, const N& ng, F f) {
    std::common_type_t<M, N> _ok = ok, _ng = ng;
    for (int i = 0; i < 100; i++) {
        std::common_type_t<M, N> mid = (_ok + _ng) / 2;
        if (f(mid)) {
            _ok = mid;
        } else {
            _ng = mid;
        }
    }
    return _ok;
}
