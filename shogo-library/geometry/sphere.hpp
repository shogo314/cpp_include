#pragma once

/**
 * @file sphere.hpp
 * @brief 球 (直線との交点、球どうしの交わり、共通部分の体積、大円距離)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <istream>
#include <optional>
#include <type_traits>
#include <vector>

#include "line.hpp"
#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/geometry/sphere.hpp

/**
 * @brief 球
 *
 * @tparam T 座標と半径の型 省略したらdouble
 */
template <typename T = double>
struct Sphere {
    Vector<T, 3> c;  //!< 中心
    T r;             //!< 半径

    constexpr Sphere() : c(), r() {}
    constexpr Sphere(const Vector<T, 3>& c, T r) : c(c), r(r) {}

    /**
     * @brief "x y z r" の順に読み込む
     */
    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, Sphere& rhs) {
        return is >> rhs.c >> rhs.r;
    }
};

/**
 * @brief 空間内の円 (sphere_sphere_intersection の戻り値)
 */
template <typename T>
struct SpaceCircle {
    Vector<T, 3> c;  //!< 中心
    Vector<T, 3> n;  //!< 円を含む平面の単位法線ベクトル
    T r;             //!< 半径 (球が接するなら 0)
};

/**
 * @brief 球 s と直線 p1p2 の交点 (0〜2個、座標の辞書順)。浮動小数点型限定。p1 != p2 であること
 * @param eps 接する (交点が1個) とみなす、中心と直線の距離と半径の差
 */
template <typename T>
std::vector<Vector<T, 3>> sphere_line_intersection(const Sphere<T>& s, const Line<T, 3>& line, T eps = 1e-9) {
    const auto& [p1, p2] = line;
    static_assert(std::is_floating_point_v<T>, "sphere_line_intersection: T must be a floating point type");
    Vector<T, 3> h = projection(line, s.c);
    T d = (h - s.c).abs();
    if (d > s.r + eps) return {};
    // 接するかどうかは、中心と直線の距離と半径の差で判定する
    if (d >= s.r - eps) return {h};
    T t = std::sqrt(s.r * s.r - d * d);
    Vector<T, 3> u = (p2 - p1) / static_cast<T>((p2 - p1).abs());
    std::vector<Vector<T, 3>> result = {h - u * t, h + u * t};
    std::sort(result.begin(), result.end());
    return result;
}

/**
 * @brief 2つの球の表面が交わってできる円。浮動小数点型限定
 *
 * 交わらない (離れている、一方が他方の内部にある、同心で半径が違う) なら std::nullopt。接するなら半径 0 の円 (中心が接点)。
 * 同じ球 (中心間の距離と半径の差がともに eps 以下) は交わりが球面全体になるので assert で落とす
 * @param eps 接するとみなす、中心間の距離と半径の和・差の差
 */
template <typename T>
std::optional<SpaceCircle<T>> sphere_sphere_intersection(const Sphere<T>& s1, const Sphere<T>& s2, T eps = 1e-9) {
    static_assert(std::is_floating_point_v<T>, "sphere_sphere_intersection: T must be a floating point type");
    Vector<T, 3> v = s2.c - s1.c;
    T d = v.abs();
    if (d <= eps) {
        assert(std::abs(s1.r - s2.r) > eps && "sphere_sphere_intersection: same spheres");
        return std::nullopt;
    }
    T s = s1.r + s2.r, t = std::abs(s1.r - s2.r);
    if (d > s + eps || d < t - eps) return std::nullopt;
    // s1 の中心から、円の中心までの距離
    T a = (d * d + s1.r * s1.r - s2.r * s2.r) / (2 * d);
    Vector<T, 3> u = v / d;
    // 接するかどうかは、中心間の距離と半径の和・差で判定する
    if (d >= s - eps || d <= t + eps) return SpaceCircle<T>{s1.c + u * a, u, T(0)};
    return SpaceCircle<T>{s1.c + u * a, u, std::sqrt(std::max<T>(s1.r * s1.r - a * a, 0))};
}

/**
 * @brief 2つの球の共通部分の体積。浮動小数点型限定
 */
template <typename T>
T sphere_sphere_intersection_volume(const Sphere<T>& s1, const Sphere<T>& s2) {
    static_assert(std::is_floating_point_v<T>, "sphere_sphere_intersection_volume: T must be a floating point type");
    const T pi = std::acos(T(-1));
    T d = (s1.c - s2.c).abs();
    if (d >= s1.r + s2.r) return 0;
    if (d <= std::abs(s1.r - s2.r)) {
        T r = std::min(s1.r, s2.r);
        return pi * r * r * r * 4 / 3;
    }
    // 交わりの円を含む平面で切った、2つの球欠 (高さ h) の体積の和
    auto cap = [&](T r, T R) {
        T h = r - (d * d + r * r - R * R) / (2 * d);
        return pi * h * h * (3 * r - h) / 3;
    };
    return cap(s1.r, s2.r) + cap(s2.r, s1.r);
}

/**
 * @brief 半径 r の球面上の 2 点 (緯度 lat、経度 lon、ラジアン) の最短距離 (大円距離)。浮動小数点型限定
 */
template <typename T>
T great_circle_distance(T lat1, T lon1, T lat2, T lon2, T r = 1) {
    static_assert(std::is_floating_point_v<T>, "great_circle_distance: T must be a floating point type");
    auto point = [](T lat, T lon) { return Vector<T, 3>(std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)); };
    const Vector<T, 3> p = point(lat1, lon1), q = point(lat2, lon2);
    // acos(dot) は近い 2 点で誤差が大きいので、外積の長さと内積から角度を求める
    return r * std::atan2(static_cast<T>(cross(p, q).abs()), dot(p, q));
}
