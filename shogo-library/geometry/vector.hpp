#pragma once

/**
 * @file vector.hpp
 * @brief 幾何ベクトル
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <istream>
#include <limits>
#include <numeric>
#include <optional>
#include <ostream>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/main/src/geometry/vector.hpp

/**
 * @brief 幾何学としてのベクトル
 *
 * @tparam T 型 省略したらdouble
 * @tparam Dim 次元 省略したら2
 */
template <typename T = double, int Dim = 2, ENABLE_IF_T(std::is_scalar_v<T> && (Dim > 0))>
struct Vector {
    std::array<T, Dim> v;  //!< ベクトルの成分表示

    /**
     * @brief デフォルトコンストラクタ
     * ゼロベクトル
     */
    constexpr Vector() noexcept : v() {}
    /**
     * @brief 可変長引数によるコンストラクタ
     *
     * @tparam Args
     */
    template <typename... Args>
    constexpr Vector(Args... args) noexcept : v{args...} {}

    /**
     * @brief i番目の成分を返す
     *
     * @param i
     * @return T&
     */
    constexpr T& operator[](int i) noexcept {
        return v[i];
    }
    /**
     * @brief i番目の成分を返す
     *
     * @param i
     * @return T&
     */
    constexpr const T& operator[](int i) const noexcept {
        return v[i];
    }

    /**
     * @brief 加算代入演算子
     *
     * @param rhs
     * @return Vector&
     */
    constexpr Vector& operator+=(const Vector& rhs) noexcept {
        for (int i = 0; i < Dim; i++) v[i] += rhs[i];
        return *this;
    }
    /**
     * @brief 減算代入演算子
     *
     * @param rhs
     * @return Vector&
     */
    constexpr Vector& operator-=(const Vector& rhs) noexcept {
        for (int i = 0; i < Dim; i++) v[i] -= rhs.v[i];
        return *this;
    }
    /**
     * @brief スカラー倍代入演算子
     *
     * @param coef
     * @return Vector&
     */
    constexpr Vector& operator*=(const T& coef) noexcept {
        for (int i = 0; i < Dim; i++) v[i] *= coef;
        return *this;
    }
    /**
     * @brief スカラーの逆数倍代入演算子
     *
     * @param coef
     * @return Vector&
     */
    constexpr Vector& operator/=(const T& coef) {
        for (int i = 0; i < Dim; i++) {
            if constexpr (std::is_integral_v<T>) {
                assert(v[i] % coef == 0 && "Vector::operator/= : coef must be a divisor of all elements");
            }
            v[i] /= coef;
        }
        return *this;
    }

    /**
     * @brief 単項プラス演算子
     *
     * @return Vector
     */
    [[nodiscard]]
    constexpr Vector operator+() const noexcept {
        return *this;
    }
    /**
     * @brief 単項マイナス演算子
     *
     * @return Vector
     */
    [[nodiscard]]
    constexpr Vector operator-() const noexcept {
        return *this * (-1);
    }

    /**
     * @brief 加算演算子
     *
     * @param lhs
     * @param rhs
     * @return Vector
     */
    [[nodiscard]]
    friend constexpr Vector operator+(const Vector& lhs, const Vector& rhs) noexcept {
        return std::move(Vector(lhs) += rhs);
    }

    /**
     * @brief 減算演算子
     *
     * @param lhs
     * @param rhs
     * @return Vector
     */
    [[nodiscard]]
    friend constexpr Vector operator-(const Vector& lhs, const Vector& rhs) noexcept {
        return std::move(Vector(lhs) -= rhs);
    }

    /**
     * @brief スカラー倍演算子
     *
     * @param lhs
     * @param coef
     * @return Vector
     */
    [[nodiscard]]
    friend constexpr Vector operator*(const Vector& lhs, const T& coef) noexcept {
        return std::move(Vector(lhs) *= coef);
    }

    /**
     * @brief スカラー倍演算子
     *
     * @param coef
     * @param rhs
     * @return Vector
     */
    [[nodiscard]]
    friend constexpr Vector operator*(const T& coef, const Vector& rhs) noexcept {
        return std::move(Vector(rhs) *= coef);
    }

    /**
     * @brief スカラーの逆数倍演算子
     *
     * @param lhs
     * @param coef
     * @return Vector
     */
    [[nodiscard]]
    friend constexpr Vector operator/(const Vector& lhs, const T& coef) {
        return std::move(Vector(lhs) /= coef);
    }

    /**
     * @brief 等価演算子
     *
     * @param lhs
     * @param rhs
     * @return bool
     */
    [[nodiscard]]
    friend constexpr bool operator==(const Vector& lhs, const Vector& rhs) noexcept {
        return lhs.v == rhs.v;
    }

    /**
     * @brief 非等価演算子
     *
     * @param lhs
     * @param rhs
     * @return bool
     */
    [[nodiscard]]
    friend constexpr bool operator!=(const Vector& lhs, const Vector& rhs) noexcept {
        return lhs.v != rhs.v;
    }

    /**
     * @brief 小なり演算子
     *
     * @param lhs
     * @param rhs
     * @return bool
     */
    [[nodiscard]]
    friend constexpr bool operator<(const Vector& lhs, const Vector& rhs) noexcept {
        return lhs.v < rhs.v;
    }
    /**
     * @brief 大なり演算子
     *
     * @param lhs
     * @param rhs
     * @return bool
     */
    [[nodiscard]]
    friend constexpr bool operator>(const Vector& lhs, const Vector& rhs) noexcept {
        return lhs.v > rhs.v;
    }
    /**
     * @brief 小なりイコール演算子
     *
     * @param lhs
     * @param rhs
     * @return bool
     */
    [[nodiscard]]
    friend constexpr bool operator<=(const Vector& lhs, const Vector& rhs) noexcept {
        return lhs.v <= rhs.v;
    }
    /**
     * @brief 大なりイコール演算子
     *
     * @param lhs
     * @param rhs
     * @return bool
     */
    [[nodiscard]]
    friend constexpr bool operator>=(const Vector& lhs, const Vector& rhs) noexcept {
        return lhs.v >= rhs.v;
    }

    /**
     * @brief 入力ストリームからの読み込み
     *
     * @tparam CharT 入力ストリームの文字型
     * @tparam Traits 入力ストリームの文字型特性
     * @param is
     * @param rhs
     * @return std::basic_istream<CharT, Traits>&
     */
    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, Vector& rhs) {
        for (int i = 0; i < Dim; i++) is >> rhs.v[i];
        return is;
    }
    /**
     * @brief 出力ストリームへの書き込み
     *
     * @tparam CharT 出力ストリームの文字型
     * @tparam Traits 出力ストリームの文字型特性
     * @param is
     * @param rhs
     * @return std::basic_ostream<CharT, Traits>&
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Vector& rhs) {
        for (int i = 0; i < Dim; i++) {
            if (i != 0) os << CharT(' ');
            os << rhs.v[i];
        }
        return os;
    }

    /**
     * @brief 内積
     *
     * @param lhs
     * @param rhs
     * @return T
     */
    friend constexpr T dot(const Vector& lhs, const Vector& rhs) noexcept {
        T ret = 0;
        for (int i = 0; i < Dim; i++) ret += lhs[i] * rhs[i];
        return ret;
    }

    /**
     * @brief 絶対値の2乗
     *
     * @return T
     */
    constexpr T squared_norm() const noexcept {
        return dot(*this, *this);
    }
    /**
     * @brief 絶対値
     *
     * @return double
     */
    constexpr double abs() const noexcept {
        return std::sqrt(squared_norm());
    }
};

/**
 * @brief 二次元ベクトルどうしの外積(平行四辺形の符号付き面積)
 *
 * @param lhs
 * @param rhs
 * @tparam T 座標の型
 * @return T
 */
template <typename T>
constexpr T cross(const Vector<T, 2>& lhs, const Vector<T, 2>& rhs) noexcept {
    return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}
/**
 * @brief 三次元ベクトルどうしの外積
 *
 * @param lhs
 * @param rhs
 * @tparam T 座標の型
 * @return Vector<T, 3>
 */
template <typename T>
constexpr Vector<T, 3> cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) noexcept {
    Vector<T, 3> ret;
    ret[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
    ret[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
    ret[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
    return ret;
}

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
 * @brief 直線 p1p2 と直線 q1q2 の交点。浮動小数点型限定
 * @return std::optional<Vector<T, 2>> 交点 (平行なら std::nullopt)
 */
template <typename T>
constexpr std::optional<Vector<T, 2>> line_intersection(const Vector<T, 2>& p1, const Vector<T, 2>& p2, const Vector<T, 2>& q1, const Vector<T, 2>& q2) {
    static_assert(std::is_floating_point_v<T>, "line_intersection: T must be a floating point type");
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
    for (int i = 0; i < (int)polygon.size(); i++) {
        int j = i + 1;
        if (j == (int)polygon.size()) j = 0;
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
    for (int i = 0; i < (int)polygon.size(); i++) {
        int j = i + 1;
        if (j == (int)polygon.size()) j = 0;
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
        while ((int)ret.size() >= 2) {
            T c = cross(ret.back() - ret[(int)ret.size() - 2], p - ret.back());
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
    for (int i = 1; i < (int)tmp.size() - 1; i++) ret.push_back(tmp[i]);
    return ret;
}

/**
 * @brief closest_point_pair_squared の戻り値
 */
template <typename T>
struct ClosestPairSquared {
    T squared_dist;  //!< 最近点対の距離の2乗
    int i, j;        //!< 最近点対の添字
};
/**
 * @brief closest_point_pair の戻り値
 */
struct ClosestPair {
    double dist;  //!< 最近点対の距離
    int i, j;     //!< 最近点対の添字
};

/**
 * @brief 最近点対
 *
 * @tparam T 座標の型
 * @param points 頂点リスト
 * @return ClosestPairSquared<T> 最近点対の距離の2乗と添字
 */
template <typename T>
ClosestPairSquared<T> closest_point_pair_squared(const std::vector<Vector<T, 2>>& points) {
    std::vector<int> sorted_idx(points.size());
    std::iota(sorted_idx.begin(), sorted_idx.end(), 0);
    std::sort(sorted_idx.begin(), sorted_idx.end(), [&](int i, int j) {
        return points[i][0] < points[j][0];
    });
    std::vector<int> left(1, 0);                                        // 左端
    std::vector<int> right(1, points.size());                           // 右端
    std::vector<int> mid(1, points.size() / 2);                         // 中央
    std::vector<int> par(1, -1);                                        // 親
    std::vector<T> min_dist_squared(1, std::numeric_limits<T>::max());  // 最小距離の2乗
    std::vector<std::pair<int, int>> min_dist_pair(1, {-1, -1});        // 最小距離の点対
    std::stack<int> stk;
    stk.push(~0);
    stk.push(0);
    while (!stk.empty()) {
        int u = stk.top();
        stk.pop();
        if (u >= 0) {
            // 行きがけ
            if (left[u] + 1 < mid[u]) {
                int v = left.size();
                left.push_back(left[u]);
                right.push_back(mid[u]);
                mid.push_back((left[u] + mid[u]) / 2);
                par.push_back(u);
                min_dist_squared.push_back(std::numeric_limits<T>::max());
                min_dist_pair.push_back({-1, -1});
                stk.push(~v);
                stk.push(v);
            }
            if (mid[u] + 1 < right[u]) {
                int v = left.size();
                left.push_back(mid[u]);
                right.push_back(right[u]);
                mid.push_back((mid[u] + right[u]) / 2);
                par.push_back(u);
                min_dist_squared.push_back(std::numeric_limits<T>::max());
                min_dist_pair.push_back({-1, -1});
                stk.push(~v);
                stk.push(v);
            }
        } else {
            // 帰りがけ
            u = ~u;
            std::vector<int> cand;
            for (int i = left[u]; i < right[u]; i++) {
                T dx = points[sorted_idx[i]][0] - points[sorted_idx[mid[u]]][0];
                if (dx * dx < min_dist_squared[u]) cand.push_back(i);
            }
            std::sort(cand.begin(), cand.end(), [&](int i, int j) {
                return points[sorted_idx[i]][1] < points[sorted_idx[j]][1];
            });
            for (int i = 0; i < (int)cand.size(); i++) {
                for (int j = i + 1; j < (int)cand.size(); j++) {
                    T dx = points[sorted_idx[cand[j]]][0] - points[sorted_idx[cand[i]]][0];
                    T dy = points[sorted_idx[cand[j]]][1] - points[sorted_idx[cand[i]]][1];
                    if (dy * dy >= min_dist_squared[u]) break;
                    T d = dx * dx + dy * dy;
                    if (d < min_dist_squared[u]) {
                        min_dist_squared[u] = d;
                        min_dist_pair[u] = {sorted_idx[cand[i]], sorted_idx[cand[j]]};
                    }
                }
            }
            if (par[u] != -1 && min_dist_squared[u] < min_dist_squared[par[u]]) {
                min_dist_squared[par[u]] = min_dist_squared[u];
                min_dist_pair[par[u]] = min_dist_pair[u];
            }
        }
    }
    return {min_dist_squared[0], min_dist_pair[0].first, min_dist_pair[0].second};
}

/**
 * @brief 最近点対
 *
 * @tparam T 座標の型
 * @param points 頂点リスト
 * @return ClosestPair 最近点対の距離と添字
 */
template <typename T>
ClosestPair closest_point_pair(const std::vector<Vector<T, 2>>& points) {
    auto [d2, i, j] = closest_point_pair_squared(points);
    return {std::sqrt(static_cast<double>(d2)), i, j};
}
/**
 * @deprecated closest_point_pair_squared を使うこと(綴りの誤りを直して改名した)
 */
template <typename T>
[[deprecated("use closest_point_pair_squared")]] ClosestPairSquared<T> closest_point_pair_squred(const std::vector<Vector<T, 2>>& points) {
    return closest_point_pair_squared(points);
}
