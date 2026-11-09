#pragma once

/**
 * @file predicates.hpp
 * @brief 幾何の内部計算 (整数座標の拡張、向き、ノルム)
 */

#include <cmath>
#include <type_traits>

#include "../vector.hpp"
// https://github.com/shogo314/shogo-library/blob/27998142f20ef4e094add3b53cc9c070db4ddbf8/src/geometry/internal/predicates.hpp

namespace internal_predicates {
// 整数座標の外積・体積の計算に使う型 (座標が 1e9 なら体積は 1e27 程度になる)
template <typename T>
using Wide = std::conditional_t<std::is_integral_v<T>, __int128_t, T>;
template <typename T, int Dim>
constexpr Vector<Wide<T>, Dim> widen(const Vector<T, Dim>& p) {
    Vector<Wide<T>, Dim> result;
    for (int i = 0; i < Dim; ++i) result[i] = static_cast<Wide<T>>(p[i]);
    return result;
}
// 四面体 abcd の符号付き体積の 6 倍の符号 (d が平面 abc の、abc が反時計回りに見える側なら正)
template <typename T>
int orient(const Vector<T, 3>& a, const Vector<T, 3>& b, const Vector<T, 3>& c, const Vector<T, 3>& d) {
    const auto wa = widen(a);
    const auto v = dot(cross(widen(b) - wa, widen(c) - wa), widen(d) - wa);
    return (v > 0) - (v < 0);
}
template <typename T>
double norm(const Vector<T, 3>& v) {
    const double x = static_cast<double>(v[0]), y = static_cast<double>(v[1]), z = static_cast<double>(v[2]);
    return std::sqrt(x * x + y * y + z * z);
}
}  // namespace internal_predicates
