#pragma once

/**
 * @file binary-search.hpp
 * @brief 二分探索
 */

#include <cmath>
#include <type_traits>

#include "traits.hpp"
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/utility/binary-search.hpp

/**
 * f(ok) が真、f(ng) が偽として、境界で f が真になる側の値を返す
 * 整数なら ok と ng の差が 1 になるまで、浮動小数点数なら 100 回二分する
 */
template <typename M, typename N, class F, ENABLE_IF_T(std::is_integral_v<std::common_type_t<M, N>>and std::is_invocable_r_v<bool, F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(const M& ok, const N& ng, F f) {
    std::common_type_t<M, N> _ok = ok, _ng = ng;
    while (std::abs(_ok - _ng) > 1) {
        std::common_type_t<M, N> mid = (_ok + _ng) / 2;
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
