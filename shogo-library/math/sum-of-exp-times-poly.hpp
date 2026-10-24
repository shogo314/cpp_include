#pragma once

/**
 * @file sum-of-exp-times-poly.hpp
 * @brief Σ r^i f(i) (f は多項式) とその無限和を O(d) で求める
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "combinatorics.hpp"
#include "lagrange-interpolation.hpp"
#include "prime.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/sum-of-exp-times-poly.hpp

namespace internal_sum_of_exp_times_poly {
// 0^d, 1^d, ..., m^d を、素数は累乗、合成数は最小素因数との積で O(m) 回の掛け算で求める
template <typename mint>
std::vector<mint> powers(long long m, long long d) {
    std::vector<mint> pw(m + 1);
    if (m >= 0) pw[0] = d == 0 ? mint(1) : mint(0);
    if (m >= 1) pw[1] = 1;
    Sieve sieve(std::max<long long>(m, 1));
    for (long long i = 2; i <= m; i++) {
        long long p = sieve.min_factor(i);
        pw[i] = p == i ? mint(i).pow(d) : pw[p] * pw[i / p];
    }
    return pw;
}
// f(0), ..., f(d) (d 次以下の多項式の値) と r (!= 0, 1) について、Σ_{i<n} r^i f(i) = c - r^n g(n) の c と g(0), ..., g(d)
// s_k = Σ_{i<k} r^i f(i) = c - r^k g(k) で、g の d+1 階差分 Σ_k (-1)^{d+1-k} C(d+1, k) g(k) が 0 になることから c を求める
template <typename mint>
std::pair<mint, std::vector<mint>> constant_and_g(mint r, const std::vector<mint>& f) {
    using C = Combination<mint>;
    const long long d = static_cast<long long>(f.size()) - 1;
    C::extend(d + 1);
    const mint inv_r = r.inv();
    // s_k (k = 0, ..., d+1)
    std::vector<mint> s(d + 2, 0);
    mint rk = 1;
    for (long long k = 0; k <= d; k++) s[k + 1] = s[k] + rk * f[k], rk *= r;
    // c Σ w_k = Σ w_k s_k、w_k = (-1)^{d+1-k} C(d+1, k) r^{-k}
    mint num = 0, den = 0, ir = 1;
    for (long long k = 0; k <= d + 1; k++) {
        mint w = C::C(d + 1, k) * ir;
        if ((d + 1 - k) % 2 == 1) w = -w;
        num += w * s[k], den += w;
        ir *= inv_r;
    }
    const mint c = num / den;
    std::vector<mint> g(d + 1);
    ir = 1;
    for (long long k = 0; k <= d; k++) g[k] = (c - s[k]) * ir, ir *= inv_r;
    return {c, g};
}
}  // namespace internal_sum_of_exp_times_poly

/**
 * @brief Σ_{i=0}^{n-1} r^i f(i) (f は d 次以下の多項式で、f(0), ..., f(d) を渡す) O(d) (n は 10^18 程度でもよい)
 * 法は d + 2 より大きい素数であること。0^0 = 1 とする
 */
template <typename mint>
mint sum_of_exp_times_poly(mint r, const std::vector<mint>& f, long long n) {
    assert(!f.empty() && n >= 0);
    const long long d = static_cast<long long>(f.size()) - 1;
    if (n == 0) return mint(0);
    // r = 0 なら i = 0 の項 (r^0 = 1) だけ
    if (r == 0) return f[0];
    if (r == 1) {
        // Σ_{i<n} f(i) は n の d+1 次以下の多項式なので、n = 0, ..., d+1 での値から補間する
        std::vector<mint> s(d + 2, 0);
        for (long long k = 0; k <= d; k++) s[k + 1] = s[k] + f[k];
        return lagrange_interpolation(s, mint(n));
    }
    auto [c, g] = internal_sum_of_exp_times_poly::constant_and_g(r, f);
    return c - r.pow(n) * lagrange_interpolation(g, mint(n));
}

/**
 * @brief Σ_{i=0}^{n-1} r^i i^d O(d) (n は 10^18 程度でもよい)。法は d + 2 より大きい素数であること。0^0 = 1 とする
 */
template <typename mint>
mint sum_of_exp_times_poly(mint r, long long d, long long n) {
    assert(d >= 0);
    return sum_of_exp_times_poly(r, internal_sum_of_exp_times_poly::powers<mint>(d, d), n);
}

/**
 * @brief Σ_{i=0}^{∞} r^i f(i) (r を -1 < r < 1 の有理数とみなす。f は d 次以下の多項式で、f(0), ..., f(d) を渡す) O(d)
 * r != 1 であること。Σ_{i<n} r^i f(i) = c - r^n g(n) の c が答え
 */
template <typename mint>
mint sum_of_exp_times_poly_limit(mint r, const std::vector<mint>& f) {
    assert(!f.empty() && r != 1);
    if (r == 0) return f[0];
    return internal_sum_of_exp_times_poly::constant_and_g(r, f).first;
}

/**
 * @brief Σ_{i=0}^{∞} r^i i^d (r を -1 < r < 1 の有理数とみなす) O(d)。r != 1 であること。0^0 = 1 とする
 */
template <typename mint>
mint sum_of_exp_times_poly_limit(mint r, long long d) {
    assert(d >= 0);
    return sum_of_exp_times_poly_limit(r, internal_sum_of_exp_times_poly::powers<mint>(d, d));
}
