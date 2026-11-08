#pragma once

/**
 * @file plane.hpp
 * @brief 平面 (包含・平行判定、距離、直線との交点)
 */

#include <cassert>
#include <cmath>
#include <optional>
#include <type_traits>

#include "internal/predicates.hpp"
#include "line.hpp"
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/geometry/plane.hpp

/**
 * @brief 一直線上にない 3 点で表す平面。等値比較は点の選び方・順序によらず同じ平面かを判定する
 * @tparam T 座標の型。省略したら long long
 */
template <typename T = long long>
struct Plane {
    Vector<T, 3> p1, p2, p3;

   private:
    constexpr void assert_valid() const {
        using internal_predicates::widen;
        const auto n = cross(widen(p2) - widen(p1), widen(p3) - widen(p1));
        assert((n[0] != 0 || n[1] != 0 || n[2] != 0) && "Plane: collinear points");
    }

   public:
    constexpr Plane(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& p3) : p1(p1), p2(p2), p3(p3) {
        assert_valid();
    }
    /** @brief 点 p が平面上にあるか。浮動小数点型では誤差を考慮しない */
    constexpr bool contains(const Vector<T, 3>& p) const {
        assert_valid();
        using internal_predicates::widen;
        const auto a = widen(p1), n = cross(widen(p2) - a, widen(p3) - a);
        return dot(n, widen(p) - a) == 0;
    }
    /** @brief 直線全体が平面内にあるか。平面と平行でも離れている直線は false */
    constexpr bool contains(const Line<T, 3>& line) const {
        assert((line.p1[0] != line.p2[0] || line.p1[1] != line.p2[1] || line.p1[2] != line.p2[2]) && "Plane::contains: degenerate line");
        return contains(line.p1) && contains(line.p2);
    }
    constexpr bool operator==(const Plane& other) const {
        assert_valid();
        other.assert_valid();
        using internal_predicates::widen;
        const auto a = widen(p1), n = cross(widen(p2) - a, widen(p3) - a);
        return dot(n, widen(other.p1) - a) == 0 && dot(n, widen(other.p2) - a) == 0 && dot(n, widen(other.p3) - a) == 0;
    }
    constexpr bool operator!=(const Plane& other) const { return !(*this == other); }
};

template <typename T>
Plane(const Vector<T, 3>&, const Vector<T, 3>&, const Vector<T, 3>&) -> Plane<T>;

/** @brief 直線と平面が平行か。直線が平面に含まれる場合も true */
template <typename T>
constexpr bool is_parallel(const Line<T, 3>& line, const Plane<T>& plane) {
    using internal_predicates::widen;
    const auto d = widen(line.p2) - widen(line.p1);
    const auto a = widen(plane.p1), n = cross(widen(plane.p2) - a, widen(plane.p3) - a);
    assert((d[0] != 0 || d[1] != 0 || d[2] != 0) && "is_parallel: degenerate line");
    assert((n[0] != 0 || n[1] != 0 || n[2] != 0) && "is_parallel: degenerate plane");
    return dot(n, d) == 0;
}

/** @brief 平面と直線が平行か。直線が平面に含まれる場合も true */
template <typename T>
constexpr bool is_parallel(const Plane<T>& plane, const Line<T, 3>& line) {
    return is_parallel(line, plane);
}

/** @brief 平面同士が平行か。一致する場合も true */
template <typename T>
constexpr bool is_parallel(const Plane<T>& first, const Plane<T>& second) {
    using internal_predicates::widen;
    const auto a = widen(first.p1), n = cross(widen(first.p2) - a, widen(first.p3) - a);
    const auto b = widen(second.p1), u = widen(second.p2) - b, v = widen(second.p3) - b;
    const auto m = cross(u, v);
    assert((n[0] != 0 || n[1] != 0 || n[2] != 0) && "is_parallel: degenerate plane");
    assert((m[0] != 0 || m[1] != 0 || m[2] != 0) && "is_parallel: degenerate plane");
    // 法線同士の外積 (座標の差の 4 乗) を避け、3 乗までの計算で判定する。
    return dot(n, u) == 0 && dot(n, v) == 0;
}

/**
 * @brief 点 p と平面 abc の距離 (整数型でも使える)。a, b, c は一直線上にないこと
 */
template <typename T>
double distance_point_plane(const Plane<T>& plane, const Vector<T, 3>& p) {
    const auto& [a, b, c] = plane;
    using namespace internal_predicates;
    const auto wa = widen(a);
    const auto n = cross(widen(b) - wa, widen(c) - wa);
    assert(n != decltype(n)() && "distance_point_plane: degenerate plane");
    return std::abs(static_cast<double>(dot(n, widen(p) - wa))) / norm(n);
}
/**
 * @brief 直線 p1p2 と平面 abc の交点。浮動小数点型限定。p1 != p2 で、a, b, c は一直線上にないこと
 * @return std::optional<Vector<T, 3>> 交点 (平行 (平面に含まれる場合も含む) なら std::nullopt)
 */
template <typename T>
std::optional<Vector<T, 3>> line_plane_intersection(const Line<T, 3>& object, const Plane<T>& plane) {
    const auto& [p1, p2] = object;
    const auto& [a, b, c] = plane;
    static_assert(std::is_floating_point_v<T>, "line_plane_intersection: T must be a floating point type");
    assert(p1 != p2 && "line_plane_intersection: degenerate line");
    const Vector<T, 3> n = cross(b - a, c - a), d = p2 - p1;
    assert((n != Vector<T, 3>()) && "line_plane_intersection: degenerate plane");
    const T den = dot(n, d);
    if (den == 0) return std::nullopt;
    // p1 + d t が平面上にある t
    return p1 + d * (dot(n, a - p1) / den);
}
