#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/geometry/angle.hpp

/**
 * @file angle.hpp
 * @brief 偏角
 */

#include <cmath>
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>

/**
 * @brief 2 次元ベクトル (x, y) の偏角
 *
 * x 軸正方向を 0 として反時計回りに測り、周回数 turn と合わせて 2π * turn + [0, 2π) の値として比べる。零ベクトルは x 軸正方向と同じ偏角として扱う
 * 外積で比べるので、整数型なら誤差なく比べられる (座標の絶対値の 2 乗が T に収まる必要がある)
 *
 * std::sort(ps.begin(), ps.end(), [](const auto& a, const auto& b) { return Angle(a) < Angle(b); });
 *
 * @tparam T 座標の型
 */
template <typename T>
struct Angle {
    T x;                 //!< x 座標
    T y;                 //!< y 座標
    long long turn = 0;  //!< 周回数

    /**
     * @brief 偏角 0 (零ベクトル)
     */
    constexpr Angle() noexcept : x(), y() {}
    /**
     * @brief ベクトル (x, y) の偏角に 2π * turn を足したもの
     */
    constexpr Angle(T x, T y, long long turn = 0) noexcept : x(x), y(y), turn(turn) {}
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
     * @brief 周回数を除いた偏角が [0, π) なら 0、[π, 2π) なら 1
     */
    constexpr int half() const noexcept { return y < 0 || (y == 0 && x < 0); }
    /**
     * @brief 偏角 (ラジアン、2π * turn + [0, 2π))
     */
    double radian() const {
        const double pi = std::acos(-1.0);
        double t = std::atan2(static_cast<double>(y), static_cast<double>(x));
        return (t < 0 ? t + 2 * pi : t) + 2 * pi * static_cast<double>(turn);
    }
    /**
     * @brief 2π を足した偏角
     */
    constexpr Angle next_turn() const noexcept { return Angle(x, y, turn + 1); }
    /**
     * @brief π を足した偏角 (逆向きのベクトル)
     */
    constexpr Angle opposite() const noexcept { return Angle(-x, -y, turn + half()); }

    /**
     * @brief 符号を反転した偏角 (x 軸について折り返したベクトル)
     */
    constexpr Angle operator-() const noexcept {
        // 周回数を除いた偏角が 0 なら -2π * turn、そうでなければ 2π を足して [0, 2π) に収める
        bool on_zero = y == 0 && x >= 0;
        return Angle(x, -y, -turn - !on_zero);
    }
    /**
     * @brief 偏角の和 (複素数としての積)
     *
     * 座標は a と b の座標の積の大きさになる。零ベクトルは偏角 0 として扱い、もう一方のベクトルを返す
     */
    friend constexpr Angle operator+(const Angle& a, const Angle& b) noexcept {
        if (a.is_zero()) return Angle(b.x, b.y, a.turn + b.turn);
        if (b.is_zero()) return Angle(a.x, a.y, a.turn + b.turn);
        Angle c(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x, a.turn + b.turn);
        // 周回数を除いた偏角の和が 2π 以上なら繰り上げる
        // 両方 [π, 2π) なら必ず、片方だけなら和は [π, 3π) なので結果が [0, π) のとき
        if (a.half() && b.half()) c.turn++;
        if (a.half() != b.half() && c.half() == 0) c.turn++;
        return c;
    }
    /**
     * @brief 偏角の差 (a から b へ反時計回りに回す角は b - a)
     */
    friend constexpr Angle operator-(const Angle& a, const Angle& b) noexcept { return a + (-b); }
    constexpr Angle& operator+=(const Angle& rhs) noexcept { return *this = *this + rhs; }
    constexpr Angle& operator-=(const Angle& rhs) noexcept { return *this = *this - rhs; }

    friend constexpr bool operator<(const Angle& a, const Angle& b) noexcept {
        if (a.turn != b.turn) return a.turn < b.turn;
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

    /**
     * @brief x, y を読む (周回数は 0 にする)
     */
    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, Angle& rhs) {
        rhs.turn = 0;
        return is >> rhs.x >> rhs.y;
    }
    /**
     * @brief x, y を空白区切りで書く (周回数は書かない)
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Angle& rhs) {
        return os << rhs.x << CharT(' ') << rhs.y;
    }
};

template <typename P, typename = decltype(std::declval<const P&>()[0])>
Angle(const P&) -> Angle<std::remove_cv_t<std::remove_reference_t<decltype(std::declval<const P&>()[0])>>>;
template <typename T>
Angle(const std::pair<T, T>&) -> Angle<T>;
