#pragma once

/**
 * @file gaussian-integer.hpp
 * @brief ガウス整数 (a + bi、a と b は整数) と二平方和
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "modular-arithmetic.hpp"
#include "prime.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/gaussian-integer.hpp

/**
 * @brief ガウス整数 a + bi
 * @tparam T 成分の型。積やノルムで成分の 2 乗程度の値を扱うので、それがあふれない型にする
 */
template <typename T = long long>
struct GaussianInteger {
    T a = 0, b = 0;  //!< 実部と虚部

    constexpr GaussianInteger() = default;
    constexpr GaussianInteger(T a, T b = 0) : a(a), b(b) {}

    /// @brief ノルム a^2 + b^2
    constexpr T norm() const { return a * a + b * b; }
    /// @brief 共役 a - bi
    constexpr GaussianInteger conj() const { return {a, -b}; }

    friend constexpr GaussianInteger operator+(const GaussianInteger& x, const GaussianInteger& y) { return {x.a + y.a, x.b + y.b}; }
    friend constexpr GaussianInteger operator-(const GaussianInteger& x, const GaussianInteger& y) { return {x.a - y.a, x.b - y.b}; }
    friend constexpr GaussianInteger operator*(const GaussianInteger& x, const GaussianInteger& y) {
        return {x.a * y.a - x.b * y.b, x.a * y.b + x.b * y.a};
    }
    /// @brief x / y の実部と虚部をそれぞれ最も近い整数に丸めた商 (余り x - qy のノルムは y のノルムの半分以下)
    friend constexpr GaussianInteger operator/(const GaussianInteger& x, const GaussianInteger& y) {
        assert(y.norm() != 0);
        const GaussianInteger num = x * y.conj();
        const T d = y.norm();
        // 最も近い整数 floor((2n + d) / (2d))
        auto round_div = [&](T n) {
            T u = 2 * n + d, v = 2 * d;
            return u >= 0 ? u / v : -((-u + v - 1) / v);
        };
        return {round_div(num.a), round_div(num.b)};
    }
    friend constexpr GaussianInteger operator%(const GaussianInteger& x, const GaussianInteger& y) { return x - x / y * y; }
    friend constexpr bool operator==(const GaussianInteger& x, const GaussianInteger& y) { return x.a == y.a && x.b == y.b; }
    friend constexpr bool operator!=(const GaussianInteger& x, const GaussianInteger& y) { return !(x == y); }
};

/**
 * @brief ガウス整数の最大公約数の 1 つ (単数 ±1, ±i 倍の違いを除いて決まる) O(log ノルム)
 */
template <typename T>
constexpr GaussianInteger<T> gaussian_gcd(GaussianInteger<T> x, GaussianInteger<T> y) {
    while (y != GaussianInteger<T>(0)) {
        x = x % y;
        std::swap(x, y);
    }
    return x;
}

/**
 * @brief n = a^2 + b^2 となる 0 以上の整数の組 (a, b) を、a の昇順にすべて返す
 * n を素因数分解し、4 で割って 1 余る素数 p を p = π π' (π' は π の共役) と分けて、それぞれの選び方をすべて試す
 * 組の個数を K として、O(factorize(n) + p の個数 × log^2 n + K)
 * @pre 0 <= n <= 10^18 程度 (__int128 であふれない範囲)
 */
inline std::vector<std::pair<long long, long long>> two_square_sum(long long n) {
    assert(n >= 0);
    if (n == 0) return {{0, 0}};
    using G = GaussianInteger<__int128>;
    std::vector<G> cand = {G(1)};
    for (auto [p, e] : factorize(n)) {
        if (p % 4 == 3) {
            // p はガウス整数でも素数なので、偶数乗でなければ表せない
            if (e % 2 == 1) return {};
            G q(1);
            for (long long i = 0; i < e / 2; i++) q = q * G(p);
            for (auto& z : cand) z = z * q;
            continue;
        }
        // p = π π'
        G pi;
        if (p == 2) {
            pi = G(1, 1);
        } else {
            const long long t = sqrt_mod(p - 1, p);  // t^2 ≡ -1 (mod p)
            pi = gaussian_gcd(G(p), G(t, 1));
        }
        // π^j π'^(e-j) (j = 0, ..., e)。p = 2 なら π と π' は単数倍の違いなので 1 通り
        std::vector<G> pw = {G(1)}, cpw = {G(1)};
        for (long long i = 0; i < e; i++) pw.push_back(pw.back() * pi), cpw.push_back(cpw.back() * pi.conj());
        std::vector<G> next;
        for (const auto& z : cand) {
            for (long long j = 0; j <= (p == 2 ? 0 : e); j++) next.push_back(z * (p == 2 ? pw[e] : pw[j] * cpw[e - j]));
        }
        cand.swap(next);
    }
    // 単数倍して実部と虚部が 0 以上のものを集める
    std::vector<std::pair<long long, long long>> res;
    for (auto z : cand) {
        for (int u = 0; u < 4; u++, z = z * G(0, 1)) {
            if (z.a >= 0 && z.b >= 0) res.emplace_back(static_cast<long long>(z.a), static_cast<long long>(z.b));
        }
    }
    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());
    return res;
}
