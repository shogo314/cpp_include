#pragma once

/**
 * @file triangle.hpp
 * @brief 2 次元・3 次元の三角形
 */

#include <cassert>
#include <type_traits>

#include "internal/predicates.hpp"
#include "segment.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/geometry/triangle.hpp

/**
 * @brief 一直線上にない 3 頂点で表す三角形。等値比較では頂点の順序を無視する
 * @tparam T 座標の型。省略したら long long
 * @tparam Dim 次元 (2 または 3)。省略したら 2
 */
template <typename T = long long, int Dim = 2>
struct Triangle {
    static_assert(Dim == 2 || Dim == 3, "Triangle: dimension must be 2 or 3");
    Vector<T, Dim> p1, p2, p3;

   private:
    constexpr void assert_valid() const {
        using internal_predicates::widen;
        const auto a = widen(p1), u = widen(p2) - a, v = widen(p3) - a;
        if constexpr (Dim == 2) {
            assert(cross(u, v) != 0 && "Triangle: collinear vertices");
        } else {
            const auto n = cross(u, v);
            assert((n[0] != 0 || n[1] != 0 || n[2] != 0) && "Triangle: collinear vertices");
        }
    }

   public:
    constexpr Triangle(const Vector<T, Dim>& p1, const Vector<T, Dim>& p2, const Vector<T, Dim>& p3) : p1(p1), p2(p2), p3(p3) {
        assert_valid();
    }
    constexpr bool operator==(const Triangle& other) const {
        assert_valid();
        other.assert_valid();
        auto has_vertex = [&](const Vector<T, Dim>& p) { return p == other.p1 || p == other.p2 || p == other.p3; };
        return has_vertex(p1) && has_vertex(p2) && has_vertex(p3);
    }
    constexpr bool operator!=(const Triangle& other) const { return !(*this == other); }
};

template <typename T, int Dim>
Triangle(const Vector<T, Dim>&, const Vector<T, Dim>&, const Vector<T, Dim>&) -> Triangle<T, Dim>;

/**
 * @brief 線分 p1p2 と三角形 abc (周を含む) が共有点を持つか (整数型でも使える。整数座標なら正確)
 * 三角形は退化していない (a, b, c が一直線上にない) こと
 */
template <typename T>
bool segment_triangle_intersect(const Segment<T, 3>& object, const Triangle<T, 3>& triangle) {
    const auto& [p1, p2] = object;
    const auto& [a, b, c] = triangle;
    using internal_predicates::orient;
    using internal_predicates::widen;
    const auto n = cross(widen(b) - widen(a), widen(c) - widen(a));
    assert(n != decltype(n)() && "segment_triangle_intersect: degenerate triangle");
    const int s1 = orient(a, b, c, p1), s2 = orient(a, b, c, p2);
    if (s1 * s2 > 0) return false;
    if (s1 != 0 || s2 != 0) {
        // 線分が平面を貫くか片方の端点が平面上にある。直線 p1p2 から見て、3 辺が同じ向きに回っていれば三角形を通る
        const int o1 = orient(p1, p2, a, b), o2 = orient(p1, p2, b, c), o3 = orient(p1, p2, c, a);
        return !((o1 < 0 || o2 < 0 || o3 < 0) && (o1 > 0 || o2 > 0 || o3 > 0));
    }
    // 同じ平面上: 法線の成分が最も大きい軸を落として 2 次元で判定する
    int k = 0;
    for (int i = 1; i < 3; i++) {
        if ((n[i] < 0 ? -n[i] : n[i]) > (n[k] < 0 ? -n[k] : n[k])) k = i;
    }
    const int i0 = k == 0 ? 1 : 0, i1 = k == 2 ? 1 : 2;
    auto flat = [&](const Vector<T, 3>& p) { return Vector<T, 2>(p[i0], p[i1]); };
    const Vector<T, 2> q1 = flat(p1), q2 = flat(p2), ta = flat(a), tb = flat(b), tc = flat(c);
    // 端点 q1 が三角形の内部 (周を含む) にあるか、線分が辺と交わる (q1 が外で q2 が内なら辺と交わる)
    // 3 辺から見た向きが混ざらなければ内部。辺の延長上 (ccw が ±2) の点は、残りの 2 辺で向きが逆になる
    bool pos = false, neg = false;
    for (int o : {ccw(ta, tb, q1), ccw(tb, tc, q1), ccw(tc, ta, q1)}) pos |= o == 1, neg |= o == -1;
    if (!(pos && neg)) return true;
    if (q1 == q2) return false;
    return segments_intersect(Segment(q1, q2), Segment(ta, tb)) || segments_intersect(Segment(q1, q2), Segment(tb, tc)) || segments_intersect(Segment(q1, q2), Segment(tc, ta));
}
