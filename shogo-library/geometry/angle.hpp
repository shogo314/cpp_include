#pragma once
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/geometry/angle.hpp

/**
 * @file angle.hpp
 * @brief 偏角
 */

#include <cmath>
#include <type_traits>
#include <utility>

/**
 * @brief 2 次元ベクトル (x, y) の偏角
 *
 * x 軸正方向を 0 として反時計回りに [0, 2π) で比べる。零ベクトルは x 軸正方向と同じ偏角として扱う
 * 外積で比べるので、整数型なら誤差なく比べられる (座標の絶対値の 2 乗が T に収まる必要がある)
 *
 * std::sort(ps.begin(), ps.end(), [](const auto& a, const auto& b) { return Angle(a) < Angle(b); });
 *
 * @tparam T 座標の型
 */
template <typename T>
struct Angle {
    T x;  //!< x 座標
    T y;  //!< y 座標

    /**
     * @brief 偏角 0 (零ベクトル)
     */
    constexpr Angle() noexcept : x(), y() {}
    /**
     * @brief ベクトル (x, y) の偏角
     */
    constexpr Angle(T x, T y) noexcept : x(x), y(y) {}
    /**
     * @brief p[0], p[1] を x, y とする (Vector<T, 2> や std::array<T, 2> から作る)
     */
    template <typename P, typename = decltype(std::declval<const P&>()[0])>
    constexpr explicit Angle(const P& p) noexcept : x(p[0]), y(p[1]) {}
    /**
     * @brief p.first, p.second を x, y とする
     */
    constexpr explicit Angle(const std::pair<T, T>& p) noexcept : x(p.first), y(p.second) {}

    /**
     * @brief 零ベクトルか
     */
    constexpr bool is_zero() const noexcept { return x == 0 && y == 0; }
    /**
     * @brief 偏角が [0, π) なら 0、[π, 2π) なら 1
     */
    constexpr int half() const noexcept { return y < 0 || (y == 0 && x < 0); }
    /**
     * @brief 偏角 (ラジアン、[0, 2π))
     */
    double radian() const {
        double t = std::atan2(static_cast<double>(y), static_cast<double>(x));
        return t < 0 ? t + 2 * std::acos(-1.0) : t;
    }

    friend constexpr bool operator<(const Angle& a, const Angle& b) noexcept {
        // 零ベクトルは外積で比べられないので、偏角 0 (x 軸正方向) として先に判定する
        if (b.is_zero()) return false;                   // 偏角 0 より小さいものはない
        if (a.is_zero()) return !(b.y == 0 && b.x > 0);  // b の偏角が 0 より大きいか
        if (a.half() != b.half()) return a.half() < b.half();
        return b.x * a.y < a.x * b.y;
    }
    friend constexpr bool operator>(const Angle& a, const Angle& b) noexcept { return b < a; }
    friend constexpr bool operator<=(const Angle& a, const Angle& b) noexcept { return !(b < a); }
    friend constexpr bool operator>=(const Angle& a, const Angle& b) noexcept { return !(a < b); }
    /**
     * @brief 偏角が等しいか (ベクトルとして等しいかではない)
     */
    friend constexpr bool operator==(const Angle& a, const Angle& b) noexcept { return !(a < b) && !(b < a); }
    friend constexpr bool operator!=(const Angle& a, const Angle& b) noexcept { return !(a == b); }
};

template <typename P, typename = decltype(std::declval<const P&>()[0])>
Angle(const P&) -> Angle<std::remove_cv_t<std::remove_reference_t<decltype(std::declval<const P&>()[0])>>>;
template <typename T>
Angle(const std::pair<T, T>&) -> Angle<T>;
