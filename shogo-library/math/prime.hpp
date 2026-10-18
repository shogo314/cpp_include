#pragma once

/**
 * @file prime.hpp
 * @brief 素数 (素数判定、素因数分解、約数列挙、篩)
 */

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <numeric>
#include <utility>
#include <vector>

#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/math/prime.hpp

namespace internal_prime {

using u64 = unsigned long long;

/**
 * @brief Pollard の ρ 法 (Brent の改良) で合成数 n の 1 でも n でもない約数を 1 つ見つける
 */
inline u64 find_factor(u64 n) {
    if (n % 2 == 0) return 2;
    constexpr u64 m = 128;
    for (u64 c = 1;; c++) {
        auto f = [&](u64 x) { return (modmul(x, x, n) + c) % n; };
        auto diff = [](u64 x, u64 y) { return x > y ? x - y : y - x; };
        u64 x = 0, y = 2, ys = 2, q = 1, g = 1;
        for (u64 r = 1; g == 1; r <<= 1) {
            x = y;
            for (u64 i = 0; i < r; i++) y = f(y);
            for (u64 k = 0; k < r && g == 1; k += m) {
                ys = y;
                for (u64 i = 0; i < m && i < r - k; i++) {
                    y = f(y);
                    q = modmul(q, diff(x, y), n);
                }
                g = std::gcd(q, n);
            }
        }
        if (g == n) {
            // まとめて掛けたせいで n になったので、1 つずつ戻って確かめる
            do {
                ys = f(ys);
                g = std::gcd(diff(x, ys), n);
            } while (g == 1);
        }
        if (g != n) return g;
    }
}

}  // namespace internal_prime

/**
 * @brief n が素数か (Miller-Rabin、64 bit で決定的) O(log n)
 */
constexpr bool is_prime(long long n) {
    using namespace internal_prime;
    if (n < 2) return false;
    for (u64 p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}) {
        if (static_cast<u64>(n) % p == 0) return static_cast<u64>(n) == p;
    }
    const u64 m = n;
    u64 d = m - 1;
    int s = 0;
    while (d % 2 == 0) d /= 2, s++;
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        a %= m;
        if (a == 0) continue;
        u64 x = modpow(a, d, m);
        if (x == 1 || x == m - 1) continue;
        bool composite = true;
        for (int i = 1; i < s; i++) {
            x = modmul(x, x, m);
            if (x == m - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

/**
 * @brief n (>= 1) を素因数分解し、(素因数, 指数) を素因数の昇順に並べて返す
 * Pollard の ρ 法を使う。期待 O(n^(1/4)) 程度
 */
inline std::vector<std::pair<long long, long long>> factorize(long long n) {
    assert(n >= 1);
    std::vector<long long> ps;
    // 小さい素因数は割り算で取り除く
    for (long long p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}) {
        while (n % p == 0) {
            ps.push_back(p);
            n /= p;
        }
    }
    std::vector<long long> st;
    if (n > 1) st.push_back(n);
    while (!st.empty()) {
        long long x = st.back();
        st.pop_back();
        if (is_prime(x)) {
            ps.push_back(x);
            continue;
        }
        long long d = internal_prime::find_factor(x);
        st.push_back(d);
        st.push_back(x / d);
    }
    std::sort(ps.begin(), ps.end());
    std::vector<std::pair<long long, long long>> res;
    for (long long p : ps) {
        if (!res.empty() && res.back().first == p)
            res.back().second++;
        else
            res.emplace_back(p, 1);
    }
    return res;
}

/**
 * @brief n (>= 1) の正の約数を昇順に並べて返す
 */
inline std::vector<long long> divisors(long long n) {
    std::vector<long long> res = {1};
    for (auto [p, e] : factorize(n)) {
        const long long sz = res.size();
        long long pw = 1;
        for (long long i = 0; i < e; i++) {
            pw *= p;
            for (long long j = 0; j < sz; j++) res.push_back(res[j] * pw);
        }
    }
    std::sort(res.begin(), res.end());
    return res;
}

/**
 * @brief オイラーの φ 関数 (1 以上 n 以下で n と互いに素な整数の個数)
 */
inline long long totient(long long n) {
    long long res = n;
    for (auto [p, e] : factorize(n)) res = res / p * (p - 1);
    return res;
}

/**
 * @brief n 以下の素数を昇順に並べて返す
 * 奇数だけを区間ごとに篩う。O(n log log n)、メモリは O(√n + 素数の個数)
 */
inline std::vector<long long> enumerate_primes(long long n) {
    std::vector<long long> res;
    if (n < 2) return res;
    res.push_back(2);
    long long r = 1;
    while ((r + 1) * (r + 1) <= n) r++;
    // √n 以下の奇素数 p と、次に篩う p の倍数 (奇数)
    std::vector<char> small(r + 1, 1);
    std::vector<std::pair<long long, long long>> sp;
    for (long long i = 3; i <= r; i += 2) {
        if (!small[i]) continue;
        for (long long j = i * i; j <= r; j += 2 * i) small[j] = 0;
        sp.emplace_back(i, i * i);
    }
    // 奇数 2k+1 (1 <= k <= kmax) を、k の長さ B の区間ごとに篩う
    constexpr int B = 1 << 16;
    std::vector<char> block(B);
    const long long kmax = (n - 1) / 2;
    for (long long lo = 1; lo <= kmax; lo += B) {
        const long long hi = std::min(lo + B, kmax + 1);
        std::fill(block.begin(), block.begin() + (hi - lo), 1);
        for (auto& [p, m] : sp) {
            long long k = (m - 1) / 2;
            for (; k < hi; k += p) block[k - lo] = 0;
            m = 2 * k + 1;
        }
        for (long long k = lo; k < hi; k++) {
            if (block[k - lo]) res.push_back(2 * k + 1);
        }
    }
    return res;
}

/**
 * @brief n 以下の整数の最小素因数を前計算する篩 (線形篩)
 *
 * 前計算 O(n)、素数判定 O(1)、素因数分解 O(log x)
 * 例:
 * Sieve sv(1000000);
 * sv.is_prime(x); sv.factorize(x);
 */
class Sieve {
    long long n;
    std::vector<int> lpf;  // 最小素因数 (メモリを抑えるため int で持つ)
    std::vector<long long> ps;

   public:
    explicit Sieve(long long n) : n(n), lpf(n + 1, 0) {
        assert(n >= 0);
        for (int i = 2; i <= n; i++) {
            if (lpf[i] == 0) {
                lpf[i] = i;
                ps.push_back(i);
            }
            for (long long p : ps) {
                if (p > lpf[i] || p * i > n) break;
                lpf[p * i] = p;
            }
        }
    }

    /**
     * @brief 前計算した範囲の上限 n
     */
    long long limit() const { return n; }
    /**
     * @brief x (0 <= x <= n) が素数か O(1)
     */
    bool is_prime(long long x) const {
        assert(0 <= x && x <= n);
        return x >= 2 && lpf[x] == x;
    }
    /**
     * @brief x (2 <= x <= n) の最小素因数 O(1)
     */
    long long min_factor(long long x) const {
        assert(2 <= x && x <= n);
        return lpf[x];
    }
    /**
     * @brief n 以下の素数を昇順に並べたもの
     */
    const std::vector<long long>& primes() const { return ps; }
    /**
     * @brief x (1 <= x <= n) を素因数分解し、(素因数, 指数) を素因数の昇順に並べて返す O(log x)
     */
    std::vector<std::pair<long long, long long>> factorize(long long x) const {
        assert(1 <= x && x <= n);
        std::vector<std::pair<long long, long long>> res;
        while (x > 1) {
            long long p = lpf[x], e = 0;
            while (x % p == 0) x /= p, e++;
            res.emplace_back(p, e);
        }
        return res;
    }
    /**
     * @brief x (1 <= x <= n) の正の約数を昇順に並べて返す
     */
    std::vector<long long> divisors(long long x) const {
        std::vector<long long> res = {1};
        for (auto [p, e] : factorize(x)) {
            const long long sz = res.size();
            long long pw = 1;
            for (long long i = 0; i < e; i++) {
                pw *= p;
                for (long long j = 0; j < sz; j++) res.push_back(res[j] * pw);
            }
        }
        std::sort(res.begin(), res.end());
        return res;
    }
};
