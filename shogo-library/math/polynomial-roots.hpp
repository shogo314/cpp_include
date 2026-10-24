#pragma once

/**
 * @file polynomial-roots.hpp
 * @brief 有限体上の多項式の根 (Cantor-Zassenhaus 法)
 */

#include <algorithm>
#include <random>
#include <vector>

#include "fps.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/polynomial-roots.hpp

namespace internal_polynomial_roots {
// 1 次の因子の積 g (根は相異なり 0 でない) を、根ごとに分けて res に入れる
template <typename mint>
void split(const FPS<mint>& g, std::mt19937_64& rng, std::vector<mint>& res) {
    const long long d = static_cast<long long>(g.size()) - 1;
    if (d <= 0) return;
    if (d == 1) {
        res.push_back(-g[0] / g[1]);
        return;
    }
    const long long p = mint::mod();
    while (true) {
        // (x + a)^((p-1)/2) は、x + a が平方剰余になる根では 1、ならない根では -1 になる
        mint a = rng() % p;
        FPS<mint> h = modpow(FPS<mint>{a, 1}, (p - 1) / 2, g);
        h -= 1;
        h.normalize();
        FPS<mint> c = gcd(g, h);
        c.normalize();
        const long long k = static_cast<long long>(c.size()) - 1;
        if (0 < k && k < d) {
            split(c, rng, res);
            split(g / c, rng, res);
            return;
        }
    }
}
}  // namespace internal_polynomial_roots

/**
 * @brief 多項式 f の、法 p (素数) で割った余りの中の根を、相異なるものだけ値の小さい順に返す 期待 O(n log^2 n log p)
 * f が 0 (すべての値が根) のときは空を返す
 * g = gcd(f, x^p - x) で相異なる根だけを 1 次で持つ多項式にし、ランダムな a で gcd(g, (x + a)^((p-1)/2) - 1) をとって分けていく
 */
template <typename mint>
std::vector<mint> polynomial_roots(FPS<mint> f) {
    f.normalize();
    std::vector<mint> res;
    if (f.size() <= 1) return res;
    // 0 が根なら取り除く (x^p - x との gcd では 0 も根になるが、先に外しておくと分けるときに扱いやすい)
    long long z = 0;
    while (f[z] == 0) z++;
    if (z > 0) res.push_back(mint(0)), f >>= z;
    if (f.size() > 1) {
        const long long p = mint::mod();
        FPS<mint> xp = modpow(FPS<mint>{0, 1}, p, f);
        xp -= FPS<mint>{0, 1};
        xp.normalize();
        FPS<mint> g = gcd(f, xp);
        g.normalize();
        static std::mt19937_64 rng(std::random_device{}());
        internal_polynomial_roots::split(g, rng, res);
    }
    std::sort(res.begin(), res.end(), [](const mint& a, const mint& b) { return a.val() < b.val(); });
    return res;
}
