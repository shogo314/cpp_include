#pragma once

/**
 * @file segment.hpp
 * @brief 2 次元の点・直線・線分 (交差判定、交点、射影、距離)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <type_traits>

#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/a27de20f03b13a92daaf6af229791aeead4c70c9/src/geometry/segment.hpp

/**
 * @brief 線分の交差判定
 *
 * 整数型でも使える。外積を計算するため、座標の絶対値の2乗の数倍がTに収まる必要がある
 *
 * @param p1 1本目の線分の端点1
 * @param p2 1本目の線分の端点2
 * @param q1 2本目の線分の端点1
 * @param q2 2本目の線分の端点2
 * @tparam T 座標の型
 * @return bool 共有点を持つか
 */
template <typename T>
constexpr bool segments_intersect(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
    assert(p1 != p2 && q1 != q2 && "segments_intersect: degenerate segment");
    auto sign = [](T x) { return (x > 0) - (x < 0); };
    int q1_section = sign(cross(p2 - p1, q1 - p1));  // 直線p1p2に対してq1がどの位置にあるか
    int q2_section = sign(cross(p2 - p1, q2 - p1));  // 直線p1p2に対してq2がどの位置にあるか
    int p1_section = sign(cross(q2 - q1, p1 - q1));  // 直線q1q2に対してp1がどの位置にあるか
    int p2_section = sign(cross(q2 - q1, p2 - q1));  // 直線q1q2に対してp2がどの位置にあるか
    if (q1_section == 0 && q2_section == 0) {
        // 4点が同一直線上にある場合
        return dot(q1 - p1, q2 - p1) <= 0 || dot(q1 - p2, q2 - p2) <= 0 || dot(p1 - q1, p2 - q1) <= 0;
    }
    return q1_section * q2_section <= 0 && p1_section * p2_section <= 0;
}

/**
 * @brief 線分の交差判定と共有点の計算
 *
 * 共有点の座標は一般に整数にならないため、浮動小数点型限定
 * 整数座標で交差判定だけしたい場合は segments_intersect を使う
 *
 * @param p1 1本目の線分の端点1
 * @param p2 1本目の線分の端点2
 * @param q1 2本目の線分の端点1
 * @param q2 2本目の線分の端点2
 * @tparam T 座標の型
 * @return std::optional<Vector<T, 2>> 共有点の1例 (共有点がなければ std::nullopt)
 */
template <typename T>
constexpr std::optional<Vector<T, 2>> segment_intersection(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
    static_assert(std::is_floating_point_v<T>, "segment_intersection: T must be a floating point type (use segments_intersect for integers)");
    if (!segments_intersect(p1, p2, q1, q2)) return std::nullopt;
    if (cross(p2 - p1, q1 - p1) == 0 && cross(p2 - p1, q2 - p1) == 0) {
        // 4点が同一直線上にある場合
        if (dot(q1 - p1, q2 - p1) <= 0) return p1;
        if (dot(q1 - p2, q2 - p2) <= 0) return p2;
        return q1;
    }
    T area = cross(p2 - p1, q2 - q1);          // 4点を頂点とする四角形の符号付き面積
    T partial_area = cross(q2 - q1, p1 - q1);  // 三角形q1q2p1の符号付き面積
    return p1 + (p2 - p1) * partial_area / area;
}

/**
 * @brief 3点の位置関係 (counter-clockwise)
 *
 * 整数型でも使える。浮動小数点型では誤差を考えずに比較する
 *
 * @tparam T 座標の型
 * @return int 1: a→b→c が反時計回り, -1: 時計回り, 2: c, a, b の順に一直線上, -2: a, b, c の順に一直線上, 0: c が線分 ab 上
 */
template <typename T>
constexpr int ccw(const Vector<T, 2>& a, const Vector<T, 2>& b, const Vector<T, 2>& c) {
    Vector<T, 2> u = b - a, v = c - a;
    T cr = cross(u, v);
    if (cr > 0) return 1;
    if (cr < 0) return -1;
    if (dot(u, v) < 0) return 2;
    if (u.squared_norm() < v.squared_norm()) return -2;
    return 0;
}

/**
 * @brief 直線 p1p2 と直線 q1q2 が平行か (整数型でも使える)
 */
template <typename T>
constexpr bool is_parallel(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
    return cross(p2 - p1, q2 - q1) == 0;
}
/**
 * @brief 直線 p1p2 と直線 q1q2 が直交するか (整数型でも使える)
 */
template <typename T>
constexpr bool is_orthogonal(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
    return dot(p2 - p1, q2 - q1) == 0;
}

/**
 * @brief 点 p から直線 p1p2 に下ろした垂線の足 (射影)。浮動小数点型限定
 */
template <typename T>
constexpr Vector<T, 2> projection(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& p) {
    static_assert(std::is_floating_point_v<T>, "projection: T must be a floating point type");
    assert(p1 != p2 && "projection: degenerate line");
    Vector<T, 2> d = p2 - p1;
    return p1 + d * (dot(p - p1, d) / d.squared_norm());
}
/**
 * @brief 直線 p1p2 について点 p と対称な点 (反射)。浮動小数点型限定
 */
template <typename T>
constexpr Vector<T, 2> reflection(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& p) {
    return projection(p1, p2, p) * T(2) - p;
}

/**
 * @brief 直線 p1p2 と直線 q1q2 の交点。浮動小数点型限定。p1 != p2 かつ q1 != q2 であること
 * @return std::optional<Vector<T, 2>> 交点 (平行 (重なる直線も含む) なら std::nullopt)
 */
template <typename T>
constexpr std::optional<Vector<T, 2>> line_intersection(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
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
double distance_point_line(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& p) {
    assert(p1 != p2 && "distance_point_line: degenerate line");
    return std::abs(static_cast<double>(cross(p2 - p1, p - p1))) / (p2 - p1).abs();
}
/**
 * @brief 点 p と線分 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_segment(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& p) {
    if (p1 == p2) return (p - p1).abs();
    // 垂線の足が線分の外なら、近い方の端点との距離
    if (dot(p2 - p1, p - p1) < 0) return (p - p1).abs();
    if (dot(p1 - p2, p - p2) < 0) return (p - p2).abs();
    return distance_point_line(p1, p2, p);
}
/**
 * @brief 線分 p1p2 と線分 q1q2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_segments(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
    if (p1 != p2 && q1 != q2 && segments_intersect(p1, p2, q1, q2)) return 0;
    return std::min({distance_point_segment(p1, p2, q1), distance_point_segment(p1, p2, q2),
                     distance_point_segment(q1, q2, p1), distance_point_segment(q1, q2, p2)});
}
