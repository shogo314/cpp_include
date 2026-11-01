#pragma once

/**
 * @file segment-3d.hpp
 * @brief 3 次元の幾何 (点・直線・線分・平面の距離、直線と平面の交点、線分と三角形の交差)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <type_traits>

#include "segment.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/0488eb87696db73a49e2e3add8b80014b2d7920e/src/geometry/segment-3d.hpp

namespace internal_segment_3d {
// 整数座標の外積・体積の計算に使う型 (座標が 1e9 なら体積は 1e27 程度になる)
template <typename T>
using Wide = std::conditional_t<std::is_integral_v<T>, __int128_t, T>;
template <typename T>
Vector<Wide<T>, 3> widen(const Vector<T, 3>& p) {
    return Vector<Wide<T>, 3>(p[0], p[1], p[2]);
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
}  // namespace internal_segment_3d

/**
 * @brief 点 p から直線 p1p2 に下ろした垂線の足 (射影)。浮動小数点型限定
 */
template <typename T>
constexpr Vector<T, 3> projection(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& p) {
    static_assert(std::is_floating_point_v<T>, "projection: T must be a floating point type");
    assert(p1 != p2 && "projection: degenerate line");
    Vector<T, 3> d = p2 - p1;
    return p1 + d * (dot(p - p1, d) / d.squared_norm());
}

/**
 * @brief 点 p と直線 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_line(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& p) {
    using namespace internal_segment_3d;
    assert(p1 != p2 && "distance_point_line: degenerate line");
    return norm(cross(widen(p2) - widen(p1), widen(p) - widen(p1))) / norm(p2 - p1);
}
/**
 * @brief 点 p と線分 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_segment(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& p) {
    using namespace internal_segment_3d;
    if (p1 == p2) return norm(p - p1);
    // 垂線の足が線分の外なら、近い方の端点との距離
    if (dot(widen(p2) - widen(p1), widen(p) - widen(p1)) < 0) return norm(p - p1);
    if (dot(widen(p1) - widen(p2), widen(p) - widen(p2)) < 0) return norm(p - p2);
    return distance_point_line(p1, p2, p);
}
/**
 * @brief 直線 p1p2 と直線 q1q2 の距離 (整数型でも使える)。p1 != p2 かつ q1 != q2 であること
 */
template <typename T>
double distance_lines(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& q1, const Vector<T, 3>& q2) {
    using namespace internal_segment_3d;
    assert(p1 != p2 && q1 != q2 && "distance_lines: degenerate line");
    const auto n = cross(widen(p2) - widen(p1), widen(q2) - widen(q1));
    // 平行なら、一方の直線上の点ともう一方の直線の距離
    if (n == decltype(n)()) return distance_point_line(p1, p2, q1);
    return std::abs(static_cast<double>(dot(n, widen(q1) - widen(p1)))) / norm(n);
}
/**
 * @brief 線分 p1p2 と線分 q1q2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_segments(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& q1, const Vector<T, 3>& q2) {
    double res = std::min({distance_point_segment(p1, p2, q1), distance_point_segment(p1, p2, q2),
                           distance_point_segment(q1, q2, p1), distance_point_segment(q1, q2, p2)});
    if (p1 == p2 || q1 == q2) return res;
    // 距離の 2 乗は (s, t) ∈ [0, 1]^2 で凸なので、最小は内部の停留点か境界 (端点と線分の距離) にある
    using L = long double;
    auto to = [](const Vector<T, 3>& v) { return Vector<L, 3>(v[0], v[1], v[2]); };
    const Vector<L, 3> d = to(p2) - to(p1), e = to(q2) - to(q1), r = to(p1) - to(q1);
    const L a = dot(d, d), b = dot(d, e), c = dot(e, e), f = dot(d, r), g = dot(e, r);
    const L den = a * c - b * b;
    if (den > 0) {
        // p1 + d s と q1 + e t が最も近い s, t
        const L s = (b * g - c * f) / den, t = (a * g - b * f) / den;
        if (0 <= s && s <= 1 && 0 <= t && t <= 1) res = std::min(res, static_cast<double>((r + d * s - e * t).abs()));
    }
    return res;
}

/**
 * @brief 点 p と平面 abc の距離 (整数型でも使える)。a, b, c は一直線上にないこと
 */
template <typename T>
double distance_point_plane(const Vector<T, 3>& a, const Vector<T, 3>& b, const Vector<T, 3>& c, const Vector<T, 3>& p) {
    using namespace internal_segment_3d;
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
std::optional<Vector<T, 3>> line_plane_intersection(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& a, const Vector<T, 3>& b, const Vector<T, 3>& c) {
    static_assert(std::is_floating_point_v<T>, "line_plane_intersection: T must be a floating point type");
    assert(p1 != p2 && "line_plane_intersection: degenerate line");
    const Vector<T, 3> n = cross(b - a, c - a), d = p2 - p1;
    assert((n != Vector<T, 3>()) && "line_plane_intersection: degenerate plane");
    const T den = dot(n, d);
    if (den == 0) return std::nullopt;
    // p1 + d t が平面上にある t
    return p1 + d * (dot(n, a - p1) / den);
}

/**
 * @brief 線分 p1p2 と三角形 abc (周を含む) が共有点を持つか (整数型でも使える。整数座標なら正確)
 * 三角形は退化していない (a, b, c が一直線上にない) こと
 */
template <typename T>
bool segment_triangle_intersect(const Vector<T, 3>& p1, const Vector<T, 3>& p2, const Vector<T, 3>& a, const Vector<T, 3>& b, const Vector<T, 3>& c) {
    using internal_segment_3d::orient;
    using internal_segment_3d::widen;
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
    return segments_intersect(q1, q2, ta, tb) || segments_intersect(q1, q2, tb, tc) || segments_intersect(q1, q2, tc, ta);
}
