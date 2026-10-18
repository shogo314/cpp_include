#pragma once

/**
 * @file rational.hpp
 * @brief 有理数
 */

#include <cassert>
#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <utility>

#include "../base/functions.hpp"
// https://github.com/shogo314/shogo-library/blob/cfbc779abd4bff3efa5b12de9b69f0aabc01392d/src/math/rational.hpp

/**
 * @brief 有理数 (分数)
 *
 * 常に既約で、分母は正。0 は 0/1
 * 演算の途中でも、既約にしてから掛けるのでオーバーフローしにくい。比較は T が 64 bit 以下なら __int128 で、そうでなければ連分数展開で行う
 * std::map や std::set のキーに使える
 * 例:
 * Rational<> a(1, 3), b(1, 6);
 * a + b;   // 1/2
 * a < b;   // false
 *
 * @tparam T 整数型 (long long や __int128 など)
 */
template <typename T = long long>
class Rational {
    T p, q;  // p / q (q > 0、gcd(p, q) = 1)

    static constexpr T abs_(T x) { return x < 0 ? -x : x; }
    static constexpr T gcd_(T a, T b) {
        a = abs_(a), b = abs_(b);
        while (b != 0) {
            T t = a % b;
            a = b;
            b = t;
        }
        return a;
    }
    struct Raw {};
    // すでに既約で q > 0 とわかっている場合
    constexpr Rational(T p, T q, Raw) : p(p), q(q) {}

    // a / b と c / d (b, d > 0) を比べ、小さければ負、等しければ 0、大きければ正を返す
    static constexpr int compare(T a, T b, T c, T d) {
        if constexpr (sizeof(T) <= 8) {
            __int128 x = (__int128)a * d, y = (__int128)c * b;
            return x < y ? -1 : x > y ? 1
                                      : 0;
        } else {
            // 整数部分を比べ、同じなら小数部分の逆数を比べる (大小が逆になる)
            int sign = 1;
            while (true) {
                T x = floordiv(a, b), y = floordiv(c, d);
                if (x != y) return x < y ? -sign : sign;
                a -= x * b, c -= y * d;
                if (a == 0 || c == 0) {
                    if (a == 0 && c == 0) return 0;
                    return a == 0 ? -sign : sign;
                }
                T t = a;
                a = b, b = t;
                t = c;
                c = d, d = t;
                sign = -sign;
            }
        }
    }

   public:
    /**
     * @brief 整数 n
     */
    constexpr Rational(T n = 0) : p(n), q(1) {}
    /**
     * @brief num / den (den != 0)
     */
    constexpr Rational(T num, T den) : p(num), q(den) {
        assert(den != 0 && "Rational: division by zero");
        if (q < 0) p = -p, q = -q;
        T g = gcd_(p, q);
        p /= g, q /= g;
    }

    /**
     * @brief 分子
     */
    constexpr T num() const { return p; }
    /**
     * @brief 分母 (正)
     */
    constexpr T den() const { return q; }

    /**
     * @brief 以下で最大の整数
     */
    constexpr T floor() const { return floordiv(p, q); }
    /**
     * @brief 以上で最小の整数
     */
    constexpr T ceil() const { return ceildiv(p, q); }
    /**
     * @brief 整数か
     */
    constexpr bool is_integer() const { return q == 1; }
    /**
     * @brief double に直した値
     */
    constexpr double to_double() const { return (double)p / (double)q; }
    /**
     * @brief long double に直した値
     */
    constexpr long double to_long_double() const { return (long double)p / (long double)q; }
    explicit constexpr operator double() const { return to_double(); }

    /**
     * @brief 逆数 (0 でないこと)
     */
    constexpr Rational inv() const {
        assert(p != 0 && "Rational: division by zero");
        return p > 0 ? Rational(q, p, Raw{}) : Rational(-q, -p, Raw{});
    }
    /**
     * @brief 絶対値
     */
    constexpr Rational abs() const { return Rational(abs_(p), q, Raw{}); }

    constexpr Rational operator+() const { return *this; }
    constexpr Rational operator-() const { return Rational(-p, q, Raw{}); }

    constexpr Rational& operator+=(const Rational& r) {
        // p / q + r.p / r.q を、分母の gcd で割ってから通分する
        T g = gcd_(q, r.q);
        T qg = q / g;
        T t = p * (r.q / g) + r.p * qg;
        T g2 = gcd_(t, g);
        p = t / g2;
        q = qg * (r.q / g2);
        return *this;
    }
    constexpr Rational& operator-=(const Rational& r) { return *this += -r; }
    constexpr Rational& operator*=(const Rational& r) {
        // 掛ける前に、分子と相手の分母の gcd で割る
        T g1 = gcd_(p, r.q), g2 = gcd_(r.p, q);
        p = (p / g1) * (r.p / g2);
        q = (q / g2) * (r.q / g1);
        return *this;
    }
    constexpr Rational& operator/=(const Rational& r) { return *this *= r.inv(); }

    friend constexpr Rational operator+(Rational a, const Rational& b) { return a += b; }
    friend constexpr Rational operator-(Rational a, const Rational& b) { return a -= b; }
    friend constexpr Rational operator*(Rational a, const Rational& b) { return a *= b; }
    friend constexpr Rational operator/(Rational a, const Rational& b) { return a /= b; }

    friend constexpr bool operator==(const Rational& a, const Rational& b) { return a.p == b.p && a.q == b.q; }
    friend constexpr bool operator!=(const Rational& a, const Rational& b) { return !(a == b); }
    friend constexpr bool operator<(const Rational& a, const Rational& b) { return compare(a.p, a.q, b.p, b.q) < 0; }
    friend constexpr bool operator>(const Rational& a, const Rational& b) { return b < a; }
    friend constexpr bool operator<=(const Rational& a, const Rational& b) { return !(b < a); }
    friend constexpr bool operator>=(const Rational& a, const Rational& b) { return !(a < b); }

    /**
     * @brief "p/q" の形の文字列 (整数なら "p")
     */
    std::string to_string() const {
        auto str = [](T x) {
            if (x == 0) return std::string("0");
            bool neg = x < 0;
            std::string s;
            // 負のまま 1 桁ずつ取り出す (最小値でもオーバーフローしない)
            while (x != 0) {
                T d = x % 10;
                s += char('0' + (d < 0 ? -d : d));
                x /= 10;
            }
            if (neg) s += '-';
            return std::string(s.rbegin(), s.rend());
        };
        return q == 1 ? str(p) : str(p) + "/" + str(q);
    }

    friend std::ostream& operator<<(std::ostream& os, const Rational& r) { return os << r.to_string(); }
    /**
     * @brief "p/q" または "p" の形で読む
     */
    friend std::istream& operator>>(std::istream& is, Rational& r) {
        std::string s;
        if (!(is >> s)) return is;
        auto parse = [](const std::string& t) {
            T x = 0;
            bool neg = false;
            std::size_t i = 0;
            if (i < t.size() && (t[i] == '-' || t[i] == '+')) neg = t[i++] == '-';
            for (; i < t.size(); i++) x = x * 10 + (t[i] - '0');
            return neg ? -x : x;
        };
        auto pos = s.find('/');
        if (pos == std::string::npos)
            r = Rational(parse(s));
        else
            r = Rational(parse(s.substr(0, pos)), parse(s.substr(pos + 1)));
        return is;
    }
};
