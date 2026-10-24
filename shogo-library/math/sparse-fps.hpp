#pragma once

/**
 * @file sparse-fps.hpp
 * @brief 疎な形式的冪級数の inv・log・exp・pow・sqrt (0 でない項が K 個なら先頭 n 項を O(nK))
 */

#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "combinatorics.hpp"
#include "fps.hpp"
#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/sparse-fps.hpp

namespace internal_sparse_fps {
// f の 0 でない項 (次数, 係数) を、次数 min_deg 以上のものだけ集める
template <typename mint>
std::vector<std::pair<long long, mint>> nonzero(const FPS<mint>& f, long long min_deg = 0) {
    std::vector<std::pair<long long, mint>> res;
    for (long long i = min_deg; i < static_cast<long long>(f.size()); i++)
        if (f[i] != 0) res.emplace_back(i, f[i]);
    return res;
}
// f_0 != 0 の f について、g = f^e (e は mint) の先頭 n 項。f g' = e f' g の係数を比べて 1 項ずつ求める
// f_0 n g_n = Σ_{i >= 1} f_i g_{n-i} (e i - (n - i))
template <typename mint>
FPS<mint> pow_nonzero_constant(const FPS<mint>& f, mint e, mint g0, long long n) {
    using C = Combination<mint>;
    C::extend(n);
    auto terms = nonzero(f, 1);
    const mint inv_f0 = f[0].inv();
    FPS<mint> g(n);
    if (n == 0) return g;
    g[0] = g0;
    for (long long k = 1; k < n; k++) {
        mint s = 0;
        for (auto [i, fi] : terms) {
            if (i > k) break;
            s += fi * g[k - i] * (e * i - mint(k - i));
        }
        g[k] = s * inv_f0 * C::inv(k);
    }
    return g;
}
}  // namespace internal_sparse_fps

/**
 * @brief 1 / f の先頭 n 項 (f_0 != 0)。f g = 1 から f_0 g_k = -Σ_{i >= 1} f_i g_{k-i}
 */
template <typename mint>
FPS<mint> sparse_inv(const FPS<mint>& f, long long n) {
    assert(!f.empty() && f[0] != 0);
    auto terms = internal_sparse_fps::nonzero(f, 1);
    const mint inv_f0 = f[0].inv();
    FPS<mint> g(n);
    if (n == 0) return g;
    g[0] = inv_f0;
    for (long long k = 1; k < n; k++) {
        mint s = 0;
        for (auto [i, fi] : terms) {
            if (i > k) break;
            s += fi * g[k - i];
        }
        g[k] = -s * inv_f0;
    }
    return g;
}

/**
 * @brief log f の先頭 n 項 (f_0 = 1)。f g' = f' から k g_k = k f_k - Σ_{i >= 1} f_i (k - i) g_{k-i}
 */
template <typename mint>
FPS<mint> sparse_log(const FPS<mint>& f, long long n) {
    assert(!f.empty() && f[0] == 1);
    using C = Combination<mint>;
    C::extend(n);
    auto terms = internal_sparse_fps::nonzero(f, 1);
    FPS<mint> g(n);
    for (long long k = 1; k < n; k++) {
        mint s = k < static_cast<long long>(f.size()) ? f[k] * k : mint(0);
        for (auto [i, fi] : terms) {
            if (i >= k) break;
            s -= fi * g[k - i] * (k - i);
        }
        g[k] = s * C::inv(k);
    }
    return g;
}

/**
 * @brief exp f の先頭 n 項 (f_0 = 0)。g' = f' g から k g_k = Σ_{i >= 1} i f_i g_{k-i}
 */
template <typename mint>
FPS<mint> sparse_exp(const FPS<mint>& f, long long n) {
    assert(f.empty() || f[0] == 0);
    using C = Combination<mint>;
    C::extend(n);
    auto terms = internal_sparse_fps::nonzero(f, 1);
    FPS<mint> g(n);
    if (n == 0) return g;
    g[0] = 1;
    for (long long k = 1; k < n; k++) {
        mint s = 0;
        for (auto [i, fi] : terms) {
            if (i > k) break;
            s += fi * g[k - i] * i;
        }
        g[k] = s * C::inv(k);
    }
    return g;
}

/**
 * @brief f^e の先頭 n 項 (e >= 0)。f の最初の 0 でない項 f_d x^d を外して、f_0 != 0 の場合に直す
 */
template <typename mint>
FPS<mint> sparse_pow(const FPS<mint>& f, long long e, long long n) {
    assert(e >= 0);
    if (e == 0) {
        FPS<mint> g(n);
        if (n > 0) g[0] = 1;
        return g;
    }
    long long d = 0;
    while (d < static_cast<long long>(f.size()) && f[d] == 0) d++;
    // 全部 0 か、x^(d e) が n 以上なら 0
    if (d == static_cast<long long>(f.size()) || (d > 0 && e >= (n + d - 1) / d)) return FPS<mint>(n);
    FPS<mint> g = internal_sparse_fps::pow_nonzero_constant(f >> d, mint(e), f[d].pow(e), n - d * e);
    g <<= d * e;
    g.resize(n);
    return g;
}

/**
 * @brief sqrt f の先頭 n 項 (g^2 = f)。平方根がなければ std::nullopt
 * 最初の 0 でない項 f_d x^d は、d が偶数で f_d が平方剰余であること。法は素数であること
 */
template <typename mint>
std::optional<FPS<mint>> sparse_sqrt(const FPS<mint>& f, long long n) {
    long long d = 0;
    while (d < static_cast<long long>(f.size()) && f[d] == 0) d++;
    if (d == static_cast<long long>(f.size()) || d / 2 >= n) return FPS<mint>(n);
    if (d % 2 == 1) return std::nullopt;
    long long r = sqrt_mod(f[d].val(), mint::mod());
    if (r == -1) return std::nullopt;
    FPS<mint> g = internal_sparse_fps::pow_nonzero_constant(f >> d, mint(2).inv(), mint(r), n - d / 2);
    g <<= d / 2;
    g.resize(n);
    return g;
}
