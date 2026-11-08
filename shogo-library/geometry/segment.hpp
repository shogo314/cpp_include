#pragma once

/**
 * @file segment.hpp
 * @brief 2 次元・3 次元の線分 (交差判定、共有点、距離)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <type_traits>

#include "internal/predicates.hpp"
#include "line.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/geometry/segment.hpp

/**
 * @brief 2 端点で表す線分。長さ 0 も許す。等値比較では端点の順序を無視する
 */
template <typename T = long long, int Dim = 2>
struct Segment {
    static_assert(Dim >= 2, "dimension must be at least 2");
    Vector<T, Dim> p1, p2;

    constexpr Segment() = default;
    constexpr Segment(const Vector<T, Dim>& p1, const Vector<T, Dim>& p2) : p1(p1), p2(p2) {}
    constexpr bool operator==(const Segment& other) const {
        return (p1 == other.p1 && p2 == other.p2) || (p1 == other.p2 && p2 == other.p1);
    }
    constexpr bool operator!=(const Segment& other) const { return !(*this == other); }
};

template <typename T, int Dim>
Segment(const Vector<T, Dim>&, const Vector<T, Dim>&) -> Segment<T, Dim>;

/**
 * @brief 線分の交差判定
 *
 * 整数型でも使える。外積を計算するため、座標の絶対値の2乗の数倍がTに収まる必要がある
 *
 * @param first 1 本目の線分
 * @param second 2 本目の線分
 * @tparam T 座標の型
 * @return bool 共有点を持つか
 */
template <typename T>
constexpr bool segments_intersect(const Segment<T>& first, const Segment<T>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
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
 * @param first 1 本目の線分
 * @param second 2 本目の線分
 * @tparam T 座標の型
 * @return std::optional<Vector<T, 2>> 共有点の1例 (共有点がなければ std::nullopt)
 */
template <typename T>
constexpr std::optional<Vector<T, 2>> segment_intersection(const Segment<T>& first, const Segment<T>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    static_assert(std::is_floating_point_v<T>, "segment_intersection: T must be a floating point type (use segments_intersect for integers)");
    if (!segments_intersect(first, second)) return std::nullopt;
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
 * @brief 点 p と線分 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_segment(const Segment<T>& object, const Vector<T, 2>& p) {
    const auto& [p1, p2] = object;
    if (p1 == p2) return (p - p1).abs();
    // 垂線の足が線分の外なら、近い方の端点との距離
    if (dot(p2 - p1, p - p1) < 0) return (p - p1).abs();
    if (dot(p1 - p2, p - p2) < 0) return (p - p2).abs();
    return distance_point_line(Line(p1, p2), p);
}
/**
 * @brief 線分 p1p2 と線分 q1q2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_segments(const Segment<T>& first, const Segment<T>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    if (p1 != p2 && q1 != q2 && segments_intersect(first, second)) return 0;
    return std::min({distance_point_segment(first, q1), distance_point_segment(first, q2),
                     distance_point_segment(second, p1), distance_point_segment(second, p2)});
}
/**
 * @brief 点 p と線分 p1p2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_point_segment(const Segment<T, 3>& object, const Vector<T, 3>& p) {
    const auto& [p1, p2] = object;
    using namespace internal_predicates;
    if (p1 == p2) return norm(p - p1);
    // 垂線の足が線分の外なら、近い方の端点との距離
    if (dot(widen(p2) - widen(p1), widen(p) - widen(p1)) < 0) return norm(p - p1);
    if (dot(widen(p1) - widen(p2), widen(p) - widen(p2)) < 0) return norm(p - p2);
    return distance_point_line(Line(p1, p2), p);
}
/**
 * @brief 線分 p1p2 と線分 q1q2 の距離 (整数型でも使える)
 */
template <typename T>
double distance_segments(const Segment<T, 3>& first, const Segment<T, 3>& second) {
    const auto& [p1, p2] = first;
    const auto& [q1, q2] = second;
    double res = std::min({distance_point_segment(first, q1), distance_point_segment(first, q2),
                           distance_point_segment(second, p1), distance_point_segment(second, p2)});
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
