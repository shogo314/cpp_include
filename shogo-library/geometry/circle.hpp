#pragma once

/**
 * @file circle.hpp
 * @brief 円 (位置関係、交点、接線、内接円、外接円、共通部分の面積、最小包含円)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <istream>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/geometry/circle.hpp

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

/**
 * @brief 2つの円の共通接線 (0〜4本)。浮動小数点型限定。中心が同じなら空を返す
 *
 * 各接線を (c1 との接点, c2 との接点) の組で返し、c1 との接点の座標の昇順に並べる。
 * 2つの円が接する点での接線は、2つの接点が同じ点になる (接線は中心を結ぶ直線に垂直)
 * @param eps 接する (その種類の接線が1本) とみなす、中心間の距離と半径の和・差の差
 */
template <typename T>
std::vector<std::pair<Vector<T, 2>, Vector<T, 2>>> common_tangents(const Circle<T>& c1, const Circle<T>& c2, T eps = 1e-9) {
    static_assert(std::is_floating_point_v<T>, "common_tangents: T must be a floating point type");
    Vector<T, 2> d = c2.c - c1.c;
    T d2 = d.squared_norm(), dist = std::sqrt(d2);
    if (dist <= eps) return {};
    std::vector<std::pair<Vector<T, 2>, Vector<T, 2>>> res;
    // 共通外接線は c2 の半径を r2、共通内接線は -r2 として同じ式で求める
    for (T r2 : {c2.r, -c2.r}) {
        T dr = c1.r - r2;
        if (dist < std::abs(dr) - eps) continue;
        bool touch = std::abs(dist - std::abs(dr)) <= eps;
        T h = touch ? T(0) : std::sqrt(std::max<T>(d2 - dr * dr, 0));
        for (int sign : {1, -1}) {
            // 中心から接点への単位ベクトル
            Vector<T, 2> v = (d * dr + Vector<T, 2>(-d[1], d[0]) * (h * sign)) / d2;
            res.push_back({c1.c + v * c1.r, c2.c + v * r2});
            if (touch) break;
        }
    }
    std::sort(res.begin(), res.end());
    return res;
}

/**
 * @brief 2つの円の共通部分の面積。浮動小数点型限定
 */
template <typename T>
T circle_circle_intersection_area(const Circle<T>& c1, const Circle<T>& c2) {
    static_assert(std::is_floating_point_v<T>, "circle_circle_intersection_area: T must be a floating point type");
    const T pi = std::acos(T(-1));
    T d = (c1.c - c2.c).abs();
    if (d >= c1.r + c2.r) return 0;
    if (d <= std::abs(c1.r - c2.r)) {
        T r = std::min(c1.r, c2.r);
        return pi * r * r;
    }
    // 各円の、交点を結ぶ弦で切った弓形の面積の和
    auto segment = [&](T r, T R) {
        T a = std::acos(std::clamp((d * d + r * r - R * R) / (2 * d * r), T(-1), T(1)));  // 中心角の半分
        return r * r * (a - std::sin(2 * a) / 2);
    };
    return segment(c1.r, c2.r) + segment(c2.r, c1.r);
}

/**
 * @brief 円と多角形の共通部分の面積。浮動小数点型限定 O(n)
 *
 * 円の中心と多角形の各辺がなす三角形と円の共通部分の符号付き面積を足す
 * @param polygon 頂点の座標 (反時計回りでも時計回りでもよい)。凸でなくてもよい
 */
template <typename T>
T circle_polygon_intersection_area(const Circle<T>& c, const std::vector<Vector<T, 2>>& polygon) {
    static_assert(std::is_floating_point_v<T>, "circle_polygon_intersection_area: T must be a floating point type");
    const T r = c.r;
    // 原点 (円の中心)、a、b の三角形と円の共通部分の符号付き面積
    auto triangle = [&](const Vector<T, 2>& a, const Vector<T, 2>& b) {
        // 線分 ab を円周との交点で分ける
        std::vector<Vector<T, 2>> ps = {a};
        Vector<T, 2> v = b - a;
        T A = v.squared_norm(), B = dot(a, v), C = a.squared_norm() - r * r;
        T disc = B * B - A * C;
        if (A > 0 && disc > 0) {
            T s = std::sqrt(disc);
            for (T t : {(-B - s) / A, (-B + s) / A}) {
                if (0 < t && t < 1) ps.push_back(a + v * t);
            }
        }
        ps.push_back(b);
        T res = 0;
        for (int i = 0; i + 1 < static_cast<int>(ps.size()); i++) {
            const Vector<T, 2>&p = ps[i], &q = ps[i + 1];
            // 両端が円の内部 (円周上を含む) なら区間全体が内部なので三角形、そうでなければ扇形
            // (中点で判定すると、円に接する辺を内部とみなしてしまう)
            const T lim = r * r * (1 + 1e-9);
            if (p.squared_norm() <= lim && q.squared_norm() <= lim)
                res += cross(p, q) / 2;
            else
                res += r * r * std::atan2(cross(p, q), dot(p, q)) / 2;
        }
        return res;
    };
    T res = 0;
    const int n = static_cast<int>(polygon.size());
    for (int i = 0; i < n; i++) res += triangle(polygon[i] - c.c, polygon[i + 1 == n ? 0 : i + 1] - c.c);
    return std::abs(res);
}

/**
 * @brief minimum_enclosing_circle の戻り値。円周上にあって円を決める 1 〜 3 点で表す
 * @tparam T 座標の型。整数型なら side は __int128 で正確に判定する
 */
template <typename T>
struct EnclosingCircle {
    std::vector<Vector<T, 2>> support;  //!< 円を決める点 (1 点なら半径 0、2 点ならそれを直径とする円、3 点ならその外接円)

    /**
     * @brief 点 p が円の内側なら -1、円周上なら 0、外側なら 1 (浮動小数点型では誤差を考えない)
     */
    int side(const Vector<T, 2>& p) const {
        using W = std::conditional_t<std::is_integral_v<T>, __int128, T>;
        auto sgn = [](W x) { return x > 0 ? 1 : x < 0 ? -1
                                                      : 0; };
        if (support.size() == 1) return p == support[0] ? 0 : 1;
        if (support.size() == 2) {
            // 直径の両端から見た向きの内積の符号
            const Vector<T, 2> a = support[0] - p, b = support[1] - p;
            return sgn(static_cast<W>(a[0]) * b[0] + static_cast<W>(a[1]) * b[1]);
        }
        // 外接円の内外判定 (incircle の行列式)。三角形が時計回りなら符号を反転する
        W m[3][3];
        for (int i = 0; i < 3; i++) {
            const W x = support[i][0] - p[0], y = support[i][1] - p[1];
            m[i][0] = x, m[i][1] = y, m[i][2] = x * x + y * y;
        }
        const W det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        const Vector<T, 2> u = support[1] - support[0], v = support[2] - support[0];
        const int orient = sgn(static_cast<W>(u[0]) * v[1] - static_cast<W>(u[1]) * v[0]);
        return -sgn(det) * orient;
    }
    /**
     * @brief 中心と半径 (double)
     */
    Circle<double> circle() const {
        auto to = [](const Vector<T, 2>& a) { return Vector<double, 2>(static_cast<double>(a[0]), static_cast<double>(a[1])); };
        if (support.size() == 1) return Circle<double>(to(support[0]), 0);
        if (support.size() == 2) {
            const Vector<double, 2> a = to(support[0]), b = to(support[1]);
            return Circle<double>((a + b) / 2.0, (a - b).abs() / 2);
        }
        return circumcircle(to(support[0]), to(support[1]), to(support[2]));
    }
};

/**
 * @brief 点の集合を含む最小の円 (Welzl の乱択アルゴリズム) 期待 O(n)
 * 点の順番を乱択で入れ替え、外にある点が見つかるたびに、その点を円周上に固定して作り直す
 * @pre 点は 1 つ以上
 */
template <typename T>
EnclosingCircle<T> minimum_enclosing_circle(std::vector<Vector<T, 2>> p) {
    assert(!p.empty());
    static std::mt19937_64 rng(std::random_device{}());
    std::shuffle(p.begin(), p.end(), rng);
    const long long n = p.size();
    EnclosingCircle<T> c{{p[0]}};
    for (long long i = 1; i < n; i++) {
        if (c.side(p[i]) <= 0) continue;
        c.support = {p[i]};
        for (long long j = 0; j < i; j++) {
            if (c.side(p[j]) <= 0) continue;
            c.support = {p[i], p[j]};
            for (long long k = 0; k < j; k++) {
                if (c.side(p[k]) <= 0) continue;
                c.support = {p[i], p[j], p[k]};
            }
        }
    }
    return c;
}
