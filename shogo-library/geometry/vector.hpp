#pragma once

/**
 * @file vector.hpp
 * @brief 幾何ベクトル
 */

#include <array>
#include <cassert>
#include <cmath>
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/77ebb090933789a5c7f4dc5561b48ac9861e1e0b/src/geometry/vector.hpp

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
     * 各成分を T に変換して入れる (Vector(1, 2.5) のように型が混ざっても縮小変換の警告を出さない)
     * @tparam Args
     */
    template <typename... Args>
    constexpr Vector(Args... args) noexcept : v{static_cast<T>(args)...} {}

    /**
     * @brief i番目の成分を返す
     *
     * @param i
     * @return T&
     */
    constexpr T& operator[](long long i) noexcept {
        return v[i];
    }
    /**
     * @brief i番目の成分を返す
     *
     * @param i
     * @return T&
     */
    constexpr const T& operator[](long long i) const noexcept {
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
 * @brief 成分から作るときの推論ガイド。型は成分の共通の型、次元は成分の個数にする
 *
 * Vector(2LL, 3LL) は Vector<long long, 2>、Vector(1, 2, 3) は Vector<int, 3>、Vector(0.0, 0.0) は Vector<double, 2>
 * (これがないと、テンプレート引数の省略時の値から Vector<double, 2> になってしまう)
 */
template <typename T, typename... U, ENABLE_IF_T(std::is_scalar_v<T> && (std::is_scalar_v<U> && ...))>
Vector(T, U...) -> Vector<std::common_type_t<T, U...>, 1 + sizeof...(U)>;

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
