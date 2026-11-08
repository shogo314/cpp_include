#pragma once

/**
 * @file polygon.hpp
 * @brief 多角形 (面積、重心、内外判定、凸性、凸包、凸多角形の切断)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/77ebb090933789a5c7f4dc5561b48ac9861e1e0b/src/geometry/polygon.hpp

/**
 * @brief 多角形の符号付き面積の2倍
 *
 * @tparam T 座標の型
 * @param polygon 頂点の座標
 * @return T 面積の2倍(反時計回りなら正、時計回りなら負)
 */
template <typename T>
T polygon_area_doubled(const std::vector<Vector<T, 2>>& polygon) {
    T ret = 0;
    for (int i = 0; i < static_cast<int>(polygon.size()); i++) {
        int j = i + 1;
        if (j == static_cast<int>(polygon.size())) j = 0;
        ret += cross(polygon[i], polygon[j]);
    }
    return ret;
}

/**
 * @brief 多角形の面積
 *
 * @tparam T 座標の型
 * @param polygon 頂点の座標(順番、反時計回りか時計回りかは問わない)
 * @return double 面積
 */
template <typename T>
double polygon_area(const std::vector<Vector<T, 2>>& polygon) {
    return std::abs(polygon_area_doubled(polygon)) / 2.;
}

/**
 * @brief 多角形の面積と一次モーメント (polygon_edge_moment の戻り値)
 * 辺ごとの値を足すと、多角形全体の値になる。+ と - があるので、CircularSum などの累積和に載せられる
 *
 * @tparam T 座標の型。整数型なら値は __int128_t で持つ (座標が 1e9 なら mx は 1e27 程度になる)
 */
template <typename T>
struct PolygonMoment {
    using W = std::conditional_t<std::is_integral_v<T>, __int128_t, T>;
    W area2 = 0;       //!< 符号付き面積 S の 2 倍 (反時計回りなら正)
    W mx = 0, my = 0;  //!< 一次モーメントの 6 倍 (6S gx, 6S gy。(gx, gy) は重心)

    PolygonMoment& operator+=(const PolygonMoment& rhs) {
        area2 += rhs.area2, mx += rhs.mx, my += rhs.my;
        return *this;
    }
    PolygonMoment& operator-=(const PolygonMoment& rhs) {
        area2 -= rhs.area2, mx -= rhs.mx, my -= rhs.my;
        return *this;
    }
    friend PolygonMoment operator+(PolygonMoment lhs, const PolygonMoment& rhs) { return lhs += rhs; }
    friend PolygonMoment operator-(PolygonMoment lhs, const PolygonMoment& rhs) { return lhs -= rhs; }
    friend bool operator==(const PolygonMoment& lhs, const PolygonMoment& rhs) {
        return lhs.area2 == rhs.area2 && lhs.mx == rhs.mx && lhs.my == rhs.my;
    }
    friend bool operator!=(const PolygonMoment& lhs, const PolygonMoment& rhs) { return !(lhs == rhs); }

    /**
     * @brief 符号付き面積 (反時計回りなら正)
     */
    double area() const { return static_cast<double>(area2) / 2; }
    /**
     * @brief 重心 (面積が 0 なら assert で落ちる)
     */
    Vector<double, 2> centroid() const {
        assert(area2 != 0 && "PolygonMoment::centroid: area must be nonzero");
        const double a = static_cast<double>(area2) * 3;
        return Vector<double, 2>(static_cast<double>(mx) / a, static_cast<double>(my) / a);
    }
};

/**
 * @brief 多角形の辺 p→q の寄与 (原点と p, q でできる三角形の面積と一次モーメント)
 * 全辺の寄与の和が多角形の値になる。部分多角形のクエリは、寄与の累積和 (CircularSum など) で求められる
 *
 * @tparam T 座標の型
 */
template <typename T>
PolygonMoment<T> polygon_edge_moment(const Vector<T, 2>& p, const Vector<T, 2>& q) {
    using W = typename PolygonMoment<T>::W;
    const W c = static_cast<W>(p[0]) * q[1] - static_cast<W>(p[1]) * q[0];
    return {c, (static_cast<W>(p[0]) + q[0]) * c, (static_cast<W>(p[1]) + q[1]) * c};
}

/**
 * @brief 多角形 (の領域) の重心
 *
 * @tparam T 座標の型
 * @param polygon 頂点の座標(順番、反時計回りか時計回りかは問わない。面積が 0 なら assert で落ちる)
 * @return Vector<double, 2> 重心
 */
template <typename T>
Vector<double, 2> polygon_centroid(const std::vector<Vector<T, 2>>& polygon) {
    const int n = static_cast<int>(polygon.size());
    PolygonMoment<T> m;
    for (int i = 0; i < n; i++) m += polygon_edge_moment(polygon[i], polygon[i + 1 == n ? 0 : i + 1]);
    return m.centroid();
}

/**
 * @brief 多角形の凸性判定
 *
 * @tparam T 座標の型
 * @param polygon 頂点の座標(反時計回り)
 * @return true 凸
 * @return false 凹
 */
template <typename T>
bool polygon_is_convex(const std::vector<Vector<T, 2>>& polygon) {
    int n = polygon.size();
    for (int i = 0; i < n; i++) {
        int j = i + 1;
        if (j == n) j = 0;
        int k = j + 1;
        if (k == n) k = 0;
        if (cross(polygon[j] - polygon[i], polygon[k] - polygon[j]) < 0) return false;
    }
    return true;
}

/**
 * @brief 点の多角形内包含判定
 *
 * @tparam T 座標の型
 * @param polygon 頂点の座標(反時計回り)
 * @param p 点の座標
 * @return int 0: 外部, 1: 辺上, 2: 内部
 */
template <typename T>
int polygon_contains(const std::vector<Vector<T, 2>>& polygon, const Vector<T, 2>& p) {
    bool in = false;
    for (int i = 0; i < static_cast<int>(polygon.size()); i++) {
        int j = i + 1;
        if (j == static_cast<int>(polygon.size())) j = 0;
        Vector<T> v1 = polygon[i] - p, v2 = polygon[j] - p;
        if (v1[1] > v2[1]) std::swap(v1, v2);
        T c = cross(v1, v2);
        if (v1[1] <= 0 && v2[1] > 0 && c < 0) in = !in;
        if (c == 0 && dot(v1, v2) <= 0) return 1;
    }
    return in ? 2 : 0;
}

/**
 * @brief 多角形の半凸包
 * 与えられた頂点リストの順番を保って反時計回りに凸包を構成する
 * xの昇順にソートされていれば下凸包、xの降順にソートされていれば上凸包が得られる
 *
 * @tparam T 座標の型
 * @param points 頂点リスト
 * @param include_straight 内角が180度の頂点を含むかどうか
 * @return std::vector<Vector<T, 2>> 半凸包の頂点の座標(反時計回り)
 */
template <typename T>
std::vector<Vector<T, 2>> select_convex(const std::vector<Vector<T, 2>>& points, bool include_straight = false) {
    std::vector<Vector<T, 2>> ret;
    for (auto& p : points) {
        while (static_cast<int>(ret.size()) >= 2) {
            T c = cross(ret.back() - ret[static_cast<int>(ret.size()) - 2], p - ret.back());
            if (c > 0 || (c == 0 && include_straight)) break;
            ret.pop_back();
        }
        ret.push_back(p);
    }
    return ret;
}

/**
 * @brief 多角形の凸包
 *
 * @tparam T 座標の型
 * @param points 頂点リスト
 * @param include_straight 内角が180度の頂点を含むかどうか
 * @return std::vector<Vector<T, 2>> 凸包の頂点の座標(反時計回り)
 */
template <typename T>
std::vector<Vector<T, 2>> convex_hull(const std::vector<Vector<T, 2>>& _points, bool include_straight = false) {
    std::vector<Vector<T, 2>> points = _points;
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    std::vector<Vector<T, 2>> ret = select_convex(points, include_straight);
    std::reverse(points.begin(), points.end());
    std::vector<Vector<T, 2>> tmp = select_convex(points, include_straight);
    // 全点が一直線上にある場合、下側と上側が同じ点列になるので片方だけ使う
    if (ret.size() == points.size() && tmp.size() == points.size()) return ret;
    for (int i = 1; i < static_cast<int>(tmp.size()) - 1; i++) ret.push_back(tmp[i]);
    return ret;
}

/**
 * @brief 凸多角形を直線 p1p2 で切り、直線の左側 (p1 から p2 を見て左) の部分を返す。浮動小数点型限定
 *
 * @tparam T 座標の型
 * @param polygon 凸多角形の頂点の座標 (反時計回り)
 * @return std::vector<Vector<T, 2>> 左側の凸多角形の頂点 (反時計回り)。直線上の頂点も含む。左側に何もなければ空
 */
template <typename T>
std::vector<Vector<T, 2>> convex_cut(const std::vector<Vector<T, 2>>& polygon, const Vector<T, 2>& p1, const Vector<T, 2>& p2) {
    static_assert(std::is_floating_point_v<T>, "convex_cut: T must be a floating point type");
    assert(p1 != p2 && "convex_cut: degenerate line");
    std::vector<Vector<T, 2>> res;
    const int n = static_cast<int>(polygon.size());
    for (int i = 0; i < n; i++) {
        const Vector<T, 2>&a = polygon[i], &b = polygon[i + 1 == n ? 0 : i + 1];
        T ca = cross(p2 - p1, a - p1), cb = cross(p2 - p1, b - p1);
        if (ca >= 0) res.push_back(a);
        // 辺 ab が直線をまたぐなら、交点を入れる
        if ((ca > 0 && cb < 0) || (ca < 0 && cb > 0)) res.push_back(a + (b - a) * (ca / (ca - cb)));
    }
    return res;
}
