#pragma once

/**
 * @file combinatorial-sequences.hpp
 * @brief 分割数・ベルヌーイ数・スターリング数・ベル数 (形式的冪級数で O(n log n))、大きい n の階乗
 */

#include <cassert>
#include <vector>

#include "combinatorics.hpp"
#include "convolution.hpp"
#include "fps.hpp"
#include "multipoint.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/combinatorial-sequences.hpp

/**
 * @brief 分割数 p(0), ..., p(n) (n を正の整数の和に分ける方法の数。和の順番は区別しない) O(n log n)
 * 母関数は 1 / Π_{k >= 1} (1 - x^k)。分母は五角数定理で Σ_k (-1)^k x^{k(3k-1)/2} (k は全整数) と書ける
 */
template <typename mint>
FPS<mint> partition_numbers(long long n) {
    FPS<mint> f(n + 1, 0);
    f[0] = 1;
    for (long long k = 1;; k++) {
        const long long a = k * (3 * k - 1) / 2, b = k * (3 * k + 1) / 2;
        if (a > n) break;
        const mint s = k % 2 == 1 ? mint(-1) : mint(1);
        f[a] += s;
        if (b <= n) f[b] += s;
    }
    return inv(f, n + 1);
}

/**
 * @brief ベルヌーイ数 B_0, ..., B_n (B_1 = -1/2 とする流儀) O(n log n)
 * 指数型母関数 x / (e^x - 1) を、(e^x - 1) / x = Σ x^k / (k+1)! の逆数として求める
 */
template <typename mint>
FPS<mint> bernoulli_numbers(long long n) {
    using C = Combination<mint>;
    C::extend(n + 1);
    FPS<mint> f(n + 1);
    for (long long k = 0; k <= n; k++) f[k] = C::inverse_factorial(k + 1);
    FPS<mint> g = inv(f, n + 1);
    for (long long k = 0; k <= n; k++) g[k] *= C::factorial(k);
    return g;
}

/**
 * @brief 符号付き第 1 種スターリング数 s(n, 0), ..., s(n, n) (x (x-1) ... (x-n+1) = Σ s(n, k) x^k) O(n log n)
 * 下降階乗 f_m(x) = x (x-1) ... (x-m+1) を、f_{2m}(x) = f_m(x) f_m(x - m) と f_{m+1}(x) = f_m(x) (x - m) で倍々に作る
 * 符号なし (順列を k 個の巡回に分ける方法の数) は |s(n, k)|
 */
template <typename mint>
FPS<mint> stirling_first(long long n) {
    FPS<mint> f{1};
    long long m = 0;
    for (int bit = 62; bit >= 0; bit--) {
        if (m > 0) {
            f = f * taylor_shift(f, -mint(m));
            m *= 2;
        }
        if (n >> bit & 1) {
            f = f * FPS<mint>{-mint(m), 1};
            m++;
        }
    }
    f.resize(n + 1);
    return f;
}

/**
 * @brief 第 2 種スターリング数 S(n, 0), ..., S(n, n) (n 個の区別できる玉を k 個の区別できない空でない箱に分ける方法の数) O(n log n)
 * S(n, k) = Σ_{i=0}^{k} (-1)^{k-i} i^n / (i! (k-i)!) を畳み込みで求める
 */
template <typename mint>
FPS<mint> stirling_second(long long n) {
    using C = Combination<mint>;
    C::extend(n);
    std::vector<mint> a(n + 1), b(n + 1);
    for (long long i = 0; i <= n; i++) {
        a[i] = mint(i).pow(n) * C::inverse_factorial(i);
        b[i] = i % 2 == 0 ? C::inverse_factorial(i) : -C::inverse_factorial(i);
    }
    FPS<mint> s = convolution(a, b);
    s.resize(n + 1);
    return s;
}

namespace internal_combinatorial_sequences {
// 指数型母関数 f^k / k! の係数に i! を掛けたもの (長さ n + 1)。f は定数項が 0
template <typename mint>
FPS<mint> egf_power_column(const FPS<mint>& f, long long n, long long k) {
    using C = Combination<mint>;
    C::extend(n);
    FPS<mint> res(n + 1, mint(0));
    if (k > n) return res;
    res = pow(f, k, n + 1);
    for (long long i = 0; i <= n; i++) res[i] *= C::factorial(i) * C::inverse_factorial(k);
    return res;
}
}  // namespace internal_combinatorial_sequences

/**
 * @brief 符号付き第 1 種スターリング数の列 s(0, k), s(1, k), ..., s(n, k) (k を固定して n を動かす) O(n log n)
 * 指数型母関数 Σ_n s(n, k) x^n / n! = log(1 + x)^k / k! を使う
 */
template <typename mint>
FPS<mint> stirling_first_column(long long n, long long k) {
    assert(n >= 0 && k >= 0);
    using C = Combination<mint>;
    C::extend(n + 1);
    // log(1 + x) = Σ_{i >= 1} (-1)^(i+1) x^i / i
    FPS<mint> f(n + 1, mint(0));
    for (long long i = 1; i <= n; i++) f[i] = (i % 2 == 1 ? mint(1) : -mint(1)) * C::inv(i);
    return internal_combinatorial_sequences::egf_power_column(f, n, k);
}

/**
 * @brief 第 2 種スターリング数の列 S(0, k), S(1, k), ..., S(n, k) (k を固定して n を動かす) O(n log n)
 * 指数型母関数 Σ_n S(n, k) x^n / n! = (e^x - 1)^k / k! を使う
 */
template <typename mint>
FPS<mint> stirling_second_column(long long n, long long k) {
    assert(n >= 0 && k >= 0);
    using C = Combination<mint>;
    C::extend(n);
    // e^x - 1 = Σ_{i >= 1} x^i / i!
    FPS<mint> f(n + 1, mint(0));
    for (long long i = 1; i <= n; i++) f[i] = C::inverse_factorial(i);
    return internal_combinatorial_sequences::egf_power_column(f, n, k);
}

/**
 * @brief ベル数 B_0, ..., B_n (n 個の区別できる要素を空でない部分集合に分ける方法の数) O(n log n)
 * 指数型母関数は exp(e^x - 1)
 */
template <typename mint>
FPS<mint> bell_numbers(long long n) {
    using C = Combination<mint>;
    C::extend(n);
    FPS<mint> g(n + 1, 0);
    for (long long k = 1; k <= n; k++) g[k] = C::inverse_factorial(k);
    FPS<mint> f = exp(g, n + 1);
    for (long long k = 0; k <= n; k++) f[k] *= C::factorial(k);
    return f;
}

/**
 * @brief n! を素数 p (= mint::mod()) で割った余り (n < p、n が 10^9 程度でもよい) O(sqrt(n) log n)
 *
 * v を 2 のべきで v (v + 1) > n となるようにとり、g_d(x) = (vx + 1)(vx + 2) ... (vx + d) の g_d(0), ..., g_d(d) を
 * d = 1, 2, 4, ..., v と倍にしながら標本点のずらしで求める (g_2d(x) = g_d(x) g_d(x + d / v))
 * n! = g_v(0) g_v(1) ... g_v(q - 1) × (qv + 1) ... n (q = floor(n / v))
 */
template <typename mint>
mint large_factorial(long long n) {
    assert(0 <= n && n < static_cast<long long>(mint::mod()));
    long long v = 1;
    while (v * (v + 1) <= n) v *= 2;
    const mint inv_v = mint(v).inv();
    std::vector<mint> ys = {mint(1), mint(v) + 1};  // g_1(0), g_1(1)
    for (long long d = 1; d < v; d *= 2) {
        // g_d(0), ..., g_d(2d) と g_d(x + d / v) (x = 0, ..., 2d) を掛ける
        std::vector<mint> a = ys, ext = sample_point_shift(ys, mint(d + 1), d);
        a.insert(a.end(), ext.begin(), ext.end());
        std::vector<mint> b = sample_point_shift(ys, mint(d) * inv_v, 2 * d + 1);
        for (long long x = 0; x <= 2 * d; x++) a[x] *= b[x];
        ys.swap(a);
    }
    const long long q = n / v;
    mint res = 1;
    for (long long i = 0; i < q; i++) res *= ys[i];
    for (long long i = q * v + 1; i <= n; i++) res *= mint(i);
    return res;
}
