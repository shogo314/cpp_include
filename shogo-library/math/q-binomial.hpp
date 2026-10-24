#pragma once

/**
 * @file q-binomial.hpp
 * @brief q 二項係数
 */

#include <cassert>
#include <cstdint>
#include <vector>

#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/q-binomial.hpp

/**
 * @brief q 二項係数 C(n, k)_q = [n]_q! / ([k]_q! [n-k]_q!) を素数 p で割った余り ([i]_q = 1 + q + ... + q^(i-1)、[n]_q! = [1]_q [2]_q ... [n]_q)
 *
 * q の位数を r (q^r ≡ 1 となる最小の r >= 1。q = 0, 1 なら n_max より大きいとみなす) とすると、[i]_q ≡ 0 となるのは i が r の倍数のときだけ
 * q-Lucas の定理 C(n, k)_q ≡ C(floor(n / r), floor(k / r)) C(n mod r, k mod r)_q を使い、どちらも前計算した階乗の表で求める
 * 前計算 O(n_max + log p)、問い合わせ O(1)。表は 32 ビットで持つ
 */
class QBinomial {
    long long q, p, r;
    // r 未満の i についての [i]_q! とその逆数、floor(n_max / r) 以下の i についての i! とその逆数
    std::vector<std::uint32_t> qfact, qinv, fact, finv;

    // a[i] = a[i-1] * term(i) (i = 1, ..., n) とその逆数の表を作る
    template <typename F>
    void build(long long n, F term, std::vector<std::uint32_t>& f, std::vector<std::uint32_t>& g) const {
        f.assign(n + 1, 1), g.assign(n + 1, 1);
        for (long long i = 1; i <= n; i++) f[i] = static_cast<long long>(f[i - 1]) * term(i) % p;
        g[n] = modinv(f[n], p);
        for (long long i = n; i >= 1; i--) g[i - 1] = static_cast<long long>(g[i]) * term(i) % p;
    }

   public:
    /**
     * @brief 前計算する
     * @param q 0 <= q < p
     * @param p 素数 (2^31 以下)
     * @param n_max 問い合わせる n の最大値 (n_max < p)
     */
    QBinomial(long long q, long long p, long long n_max) : q(q), p(p) {
        assert(2 <= p && p <= (1LL << 31) && 0 <= q && q < p && 0 <= n_max && n_max < p);
        // q^i と [i]_q = (1 + q + ... + q^(i-1)) を並べ、[i]_q ≡ 0 となる最初の i を r とする
        std::vector<std::uint32_t> qint(n_max + 2, 0);
        r = n_max + 1;
        long long pw = 1, sum = 0;
        for (long long i = 1; i <= n_max; i++) {
            sum = (sum + pw) % p;
            pw = pw * q % p;
            qint[i] = sum;
            if (sum == 0) {
                r = i;
                break;
            }
        }
        build(r - 1, [&](long long i) { return static_cast<long long>(qint[i]); }, qfact, qinv);
        build(n_max / r, [](long long i) { return i; }, fact, finv);
    }

    /**
     * @brief C(n, k)_q mod p (k < 0 や k > n なら 0)
     * @pre n <= n_max
     */
    long long operator()(long long n, long long k) const {
        if (k < 0 || k > n) return 0;
        const long long n1 = n / r, k1 = k / r, n0 = n % r, k0 = k % r;
        if (k0 > n0) return 0;
        long long res = static_cast<long long>(fact[n1]) * finv[k1] % p * finv[n1 - k1] % p;
        return res * qfact[n0] % p * qinv[k0] % p * qinv[n0 - k0] % p;
    }
};
