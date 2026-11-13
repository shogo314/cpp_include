#pragma once

/**
 * @file line.hpp
 * @brief 2 次元・3 次元の直線 (平行・直交、射影・反射、交点、距離)
 */

#include <cassert>
#include <cmath>
#include <optional>
#include <type_traits>

#include "internal/predicates.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/geometry/line.hpp

/**
 * @brief 2 点で表す直線。p1 != p2 が必要。等値比較は同じ直線かを判定する
 */
template <typename T = long long, int Dim = 2>
struct Line {
    static_assert(Dim >= 2, "dimension must be at least 2");
    Vector<T, Dim> p1, p2;

    constexpr Line(const Vector<T, Dim>& p1, const Vector<T, Dim>& p2) : p1(p1), p2(p2) {
        assert(p1 != p2 && "Line: degenerate line");
    }
    constexpr bool operator==(const Line& other) const {
        assert(p1 != p2 && other.p1 != other.p2 && "Line: degenerate line");
        using Wide = std::conditional_t<std::is_integral_v<T>, __int128_t, T>;
        auto on_line = [&](const Vector<T, Dim>& p) {
            for (int i = 0; i < Dim; ++i) {
                for (int j = i + 1; j < Dim; ++j) {
                    Wide di = static_cast<Wide>(p2[i]) - p1[i], dj = static_cast<Wide>(p2[j]) - p1[j];
                    Wide vi = static_cast<Wide>(p[i]) - p1[i], vj = static_cast<Wide>(p[j]) - p1[j];
                    if (di * vj != dj * vi) return false;
                }
            }
            return true;
        };
        return on_line(other.p1) && on_line(other.p2);
    }
    constexpr bool operator!=(const Line& other) const { return !(*this == other); }
};

template <typename T, int Dim>
Line(const Vector<T, Dim>&, const Vector<T, Dim>&) -> Line<T, Dim>;

/**
 * @brief 直線 p1p2 と直線 q1q2 が平行か (整数型でも使える)
 */
template <typename T>
constexpr bool is_parallel(const Line<T>& first, const Line<T>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    return cross(p2 - p1, q2 - q1) == 0;
}
/**
 * @brief 直線 p1p2 と直線 q1q2 が直交するか (整数型でも使える)
 */
template <typename T>
constexpr bool is_orthogonal(const Line<T>& first, const Line<T>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    return dot(p2 - p1, q2 - q1) == 0;
}

/**
 * @brief 点 p から直線 p1p2 に下ろした垂線の足 (射影)。浮動小数点型限定
 */
template <typename T>
constexpr Vector<T, 2> projection(const Line<T>& object, const Vector<T, 2>& p) {
    const auto& [p1, p2] = object;
    static_assert(std::is_floating_point_v<T>, "projection: T must be a floating point type");
    assert(p1 != p2 && "projection: degenerate line");
    Vector<T, 2> d = p2 - p1;
    return p1 + d * (dot(p - p1, d) / d.squared_norm());
}
/**
 * @brief 直線 p1p2 について点 p と対称な点 (反射)。浮動小数点型限定
 */
template <typename T>
constexpr Vector<T, 2> reflection(const Line<T>& object, const Vector<T, 2>& p) {
    return projection(object, p) * T(2) - p;
}

/**
 * @brief 直線 p1p2 と直線 q1q2 の交点。浮動小数点型限定。p1 != p2 かつ q1 != q2 であること
 * @return std::optional<Vector<T, 2>> 交点 (平行 (重なる直線も含む) なら std::nullopt)
 */
template <typename T>
constexpr std::optional<Vector<T, 2>> line_intersection(const Line<T>& first, const Line<T>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    static_assert(std::is_floating_point_v<T>, "line_intersection: T must be a floating point type");
    assert(p1 != p2 && q1 != q2 && "line_intersection: degenerate line");
    Vector<T, 2> d = p2 - p1, e = q2 - q1;
    T c = cross(e, d);
    if (c == 0) return std::nullopt;
    // p1 + d t が直線 q1q2 上にある t
    return p1 + d * (cross(e, q1 - p1) / c);
}

/**
 * @brief 点 p と直線 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_line(const Line<T>& object, const Vector<T, 2>& p) {
    const auto& [p1, p2] = object;
    assert(p1 != p2 && "distance_point_line: degenerate line");
    return std::abs(static_cast<double>(cross(p2 - p1, p - p1))) / (p2 - p1).abs();
}
/** @brief 3 次元の直線同士が平行か。一致する場合も true */
template <typename T>
constexpr bool is_parallel(const Line<T, 3>& first, const Line<T, 3>& second) {
    using internal_predicates::widen;
    const auto u = widen(first.p2) - widen(first.p1), v = widen(second.p2) - widen(second.p1);
    assert((u[0] != 0 || u[1] != 0 || u[2] != 0) && "is_parallel: degenerate line");
    assert((v[0] != 0 || v[1] != 0 || v[2] != 0) && "is_parallel: degenerate line");
    const auto n = cross(u, v);
    return n[0] == 0 && n[1] == 0 && n[2] == 0;
}

/**
 * @brief 点 p から直線 p1p2 に下ろした垂線の足 (射影)。浮動小数点型限定
 */
template <typename T>
constexpr Vector<T, 3> projection(const Line<T, 3>& object, const Vector<T, 3>& p) {
    const auto& [p1, p2] = object;
    static_assert(std::is_floating_point_v<T>, "projection: T must be a floating point type");
    assert(p1 != p2 && "projection: degenerate line");
    Vector<T, 3> d = p2 - p1;
    return p1 + d * (dot(p - p1, d) / d.squared_norm());
}

/**
 * @brief 点 p と直線 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_line(const Line<T, 3>& object, const Vector<T, 3>& p) {
    const auto& [p1, p2] = object;
    using namespace internal_predicates;
    assert(p1 != p2 && "distance_point_line: degenerate line");
    return norm(cross(widen(p2) - widen(p1), widen(p) - widen(p1))) / norm(p2 - p1);
}
/**
 * @brief 直線 p1p2 と直線 q1q2 の距離 (整数型でも使える)。p1 != p2 かつ q1 != q2 であること
 */
template <typename T>
double distance_lines(const Line<T, 3>& first, const Line<T, 3>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    using namespace internal_predicates;
    assert(p1 != p2 && q1 != q2 && "distance_lines: degenerate line");
    const auto n = cross(widen(p2) - widen(p1), widen(q2) - widen(q1));
    // 平行なら、一方の直線上の点ともう一方の直線の距離
    if (n == decltype(n)()) return distance_point_line(Line(p1, p2), q1);
    return std::abs(static_cast<double>(dot(n, widen(q1) - widen(p1)))) / norm(n);
}
