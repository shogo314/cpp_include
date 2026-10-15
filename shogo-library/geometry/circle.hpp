#pragma once

/**
 * @file circle.hpp
 * @brief 円 (位置関係、交点、接線、内接円、外接円)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <istream>
#include <type_traits>
#include <vector>

#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/geometry/circle.hpp

/**
 * @brief 円
 *
 * @tparam T 座標と半径の型 省略したらdouble
 */
template <typename T = double>
struct Circle {
    Vector<T, 2> c;  //!< 中心
    T r;             //!< 半径

    constexpr Circle() : c(), r() {}
    constexpr Circle(const Vector<T, 2>& c, T r) : c(c), r(r) {}

    /**
     * @brief "x y r" の順に読み込む
     */
    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, Circle& rhs) {
        return is >> rhs.c >> rhs.r;
    }
};

/**
 * @brief 2つの円の位置関係を、共通接線の本数で返す (整数型でも使える。浮動小数点型では誤差を考えずに比較する)
 * @return int 4: 離れている, 3: 外接, 2: 2点で交わる, 1: 内接, 0: 一方が他方の内部にある (2つの円が一致する場合も 1 を返す)
 */
template <typename T>
constexpr int circle_relation(const Circle<T>& c1, const Circle<T>& c2) {
    T d2 = (c1.c - c2.c).squared_norm();
    T s = c1.r + c2.r, t = c1.r - c2.r;
    if (d2 > s * s) return 4;
    if (d2 == s * s) return 3;
    if (d2 > t * t) return 2;
    if (d2 == t * t) return 1;
    return 0;
}

/**
 * @brief 三角形 abc の内接円。浮動小数点型限定
 */
template <typename T>
Circle<T> incircle(const Vector<T, 2>& a, const Vector<T, 2>& b, const Vector<T, 2>& c) {
    static_assert(std::is_floating_point_v<T>, "incircle: T must be a floating point type");
    T la = (b - c).abs(), lb = (c - a).abs(), lc = (a - b).abs();
    T s = la + lb + lc;
    assert(s > 0);
    // 中心は、各頂点を向かい合う辺の長さで重み付けした平均
    return Circle<T>((a * la + b * lb + c * lc) / s, std::abs(cross(b - a, c - a)) / s);
}

/**
 * @brief 三角形 abc の外接円。浮動小数点型限定。3点が一直線上にあってはならない
 */
template <typename T>
Circle<T> circumcircle(const Vector<T, 2>& a, const Vector<T, 2>& b, const Vector<T, 2>& c) {
    static_assert(std::is_floating_point_v<T>, "circumcircle: T must be a floating point type");
    Vector<T, 2> u = b - a, v = c - a;
    T d = cross(u, v) * 2;
    assert(d != 0 && "circumcircle: collinear points");
    T nu = u.squared_norm(), nv = v.squared_norm();
    // a を原点としたときの中心
    Vector<T, 2> o((v[1] * nu - u[1] * nv) / d, (u[0] * nv - v[0] * nu) / d);
    return Circle<T>(a + o, o.abs());
}

/**
 * @brief 円 c と直線 p1p2 の交点 (0〜2個、座標の昇順)。浮動小数点型限定
 * @param eps 接する (交点が1個) とみなす、中心と直線の距離と半径の差
 */
template <typename T>
std::vector<Vector<T, 2>> circle_line_intersection(const Circle<T>& c, const Vector<T, 2>& p1, const Vector<T, 2>& p2, T eps = 1e-9) {
    static_assert(std::is_floating_point_v<T>, "circle_line_intersection: T must be a floating point type");
    Vector<T, 2> h = projection(p1, p2, c.c);
    T d = (h - c.c).abs();
    if (d > c.r + eps) return {};
    // 接するかどうかは、中心と直線の距離と半径の差で判定する
    if (d >= c.r - eps) return {h};
    T t = std::sqrt(c.r * c.r - d * d);
    Vector<T, 2> u = (p2 - p1) / static_cast<T>((p2 - p1).abs());
    std::vector<Vector<T, 2>> res = {h - u * t, h + u * t};
    std::sort(res.begin(), res.end());
    return res;
}

/**
 * @brief 2つの円の交点 (0〜2個、座標の昇順)。浮動小数点型限定。中心が同じなら空を返す
 * @param eps 接する (交点が1個) とみなす、中心間の距離と半径の和・差の差
 */
template <typename T>
std::vector<Vector<T, 2>> circle_circle_intersection(const Circle<T>& c1, const Circle<T>& c2, T eps = 1e-9) {
    static_assert(std::is_floating_point_v<T>, "circle_circle_intersection: T must be a floating point type");
    Vector<T, 2> v = c2.c - c1.c;
    T d = v.abs();
    if (d <= eps) return {};
    T s = c1.r + c2.r, t = std::abs(c1.r - c2.r);
    if (d > s + eps || d < t - eps) return {};
    // c1 の中心から、2つの交点を結ぶ線分の中点までの距離
    T a = (d * d + c1.r * c1.r - c2.r * c2.r) / (2 * d);
    Vector<T, 2> u = v / d;
    Vector<T, 2> m = c1.c + u * a, n(-u[1], u[0]);
    // 接するかどうかは、中心間の距離と半径の和・差で判定する
    if (d >= s - eps || d <= t + eps) return {m};
    T h = std::sqrt(std::max<T>(c1.r * c1.r - a * a, 0));
    std::vector<Vector<T, 2>> res = {m + n * h, m - n * h};
    std::sort(res.begin(), res.end());
    return res;
}

/**
 * @brief 点 p から円 c に引いた接線の接点 (0〜2個、座標の昇順)。浮動小数点型限定
 *
 * p が円の内部なら空、円周上なら p だけを返す
 * @param eps p が円周上にあるとみなす誤差
 */
template <typename T>
std::vector<Vector<T, 2>> tangent_points(const Circle<T>& c, const Vector<T, 2>& p, T eps = 1e-9) {
    static_assert(std::is_floating_point_v<T>, "tangent_points: T must be a floating point type");
    Vector<T, 2> v = p - c.c;
    T d2 = v.squared_norm(), r2 = c.r * c.r;
    T d = std::sqrt(d2);
    if (std::abs(d - c.r) <= eps) return {p};
    if (d < c.r) return {};
    // 接点は c.c + (v r^2 ± v の法線 r sqrt(d^2 - r^2)) / d^2
    T s = c.r * std::sqrt(d2 - r2);
    Vector<T, 2> n(-v[1], v[0]);
    std::vector<Vector<T, 2>> res = {c.c + (v * r2 + n * s) / d2, c.c + (v * r2 - n * s) / d2};
    std::sort(res.begin(), res.end());
    return res;
}
