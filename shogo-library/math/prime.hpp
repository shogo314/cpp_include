#pragma once

/**
 * @file prime.hpp
 * @brief 素数 (素数判定、素因数分解、約数列挙、篩、原始根)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <vector>

#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/prime.hpp

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
 * @brief m (>= 1) を法とする最小の原始根 (なければ -1)
 * 原始根があるのは m = 1, 2, 4, p^k, 2p^k (p は奇素数) のとき。m = 1 なら 0 を返す
 * 素因数分解を使うので、m が 10^18 程度でも速い
 */
inline long long primitive_root(long long m) {
    assert(m >= 1);
    if (m == 1) return 0;
    if (m == 2) return 1;
    if (m == 4) return 3;
    auto fm = factorize(m);
    // m = p^k か 2 p^k (p は奇素数) か
    if (fm.size() > 2 || (fm.size() == 2 && !(fm[0].first == 2 && fm[0].second == 1)) || (fm.size() == 1 && fm[0].first == 2)) return -1;
    long long phi = m;
    for (auto [p, e] : fm) phi = phi / p * (p - 1);
    auto fphi = factorize(phi);
    // g の位数が φ(m) であること: φ(m) のどの素因数 q についても g^(φ(m)/q) が 1 でない
    for (long long g = 2;; g++) {
        if (std::gcd(g, m) != 1) continue;
        bool ok = true;
        for (auto [q, e] : fphi) {
            if (modpow(g, phi / q, m) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) return g;
    }
}

/**
 * @brief 素数 p について x^k ≡ a (mod p) となる x を 1 つ返す (なければ -1)。0^0 = 1 とする
 *
 * g = gcd(k, p - 1) として g 乗根 z を求め、x = z^(k/g の逆数 mod (p-1)/g) とする
 * g 乗根は、g の素因数 q ごとに、位数 q^s の部分群の中で Pohlig-Hellman 法 (q の部分は baby-step giant-step) を使って直す
 * O(Σ_q (s sqrt(q) + log^2 p)) に p - 1 の素因数分解を足したもの
 */
inline long long kth_root_mod(long long k, long long a, long long p) {
    assert(k >= 0 && p >= 2);
    a %= p;
    if (a < 0) a += p;
    if (k == 0) return a == 1 % p ? 1 : -1;
    if (a == 0) return 0;
    const long long m = p - 1, g = std::gcd(k, m);
    if (modpow(a, m / g, p) != 1) return -1;
    long long z = a;  // 素因数を 1 つずつ処理し、最後に z^g = a となるようにする
    for (auto [q, e] : factorize(g)) {
        long long s = 0, t = m, qe = 1;
        while (t % q == 0) t /= q, s++;
        for (long long i = 0; i < e; i++) qe *= q;
        // x1 = z^r (r q^e ≡ 1 mod t) とすると x1^(q^e) = z h (h は位数 q^s の部分群 H の元)
        const long long r = t == 1 ? 0 : modinv(qe % t, t);
        const long long x1 = modpow(z, r, p);
        const long long target = modmul(z, modinv(modpow(x1, qe, p), p), p);  // h^(-1)、H の元で q^e 乗になっている
        // H の生成元 c = w^t (w は q 乗非剰余)
        long long w = 2;
        while (modpow(w, m / q, p) == 1) w++;
        const long long c = modpow(w, t, p);
        // target = c^d となる d を q 進で 1 桁ずつ求める (Pohlig-Hellman)
        long long zeta = c;  // 位数 q の元 c^(q^(s-1))
        for (long long i = 0; i + 1 < s; i++) zeta = modpow(zeta, q, p);
        const long long B = std::sqrt(static_cast<double>(q)) + 1;
        std::unordered_map<long long, long long> baby;  // zeta^j -> j (j < B)
        for (long long j = 0, x = 1; j < B; j++, x = modmul(x, zeta, p)) baby.emplace(x, j);
        const long long giant = modinv(modpow(zeta, B, p), p);
        auto log_zeta = [&](long long y) {
            for (long long i = 0; i <= q / B + 1; i++, y = modmul(y, giant, p)) {
                auto it = baby.find(y);
                if (it != baby.end()) return i * B + it->second;
            }
            assert(false);
            return -1LL;
        };
        long long d = 0, qi = 1;
        const long long cinv = modinv(c, p);
        for (long long i = 0; i < s; i++, qi *= q) {
            // (target c^(-d))^(q^(s-1-i)) = zeta^(d の i 桁目)
            long long y = modmul(target, modpow(cinv, d, p), p);
            for (long long j = 0; j + 1 + i < s; j++) y = modpow(y, q, p);
            d += log_zeta(y) * qi;
        }
        // d は q^e の倍数になっている
        z = modmul(x1, modpow(c, d / qe, p), p);
    }
    const long long mg = m / g;
    const long long inv = mg == 1 ? 0 : modinv((k / g) % mg, mg);
    return modpow(z, inv, p);
}

/**
 * @brief テトレーション a↑↑b (a を b 個重ねたべき乗 a^a^...^a、a↑↑0 = 1) を m で割った余り。0^0 = 1 とする
 *
 * x >= log2(m) なら a^x ≡ a^(x mod φ(m) + φ(m)) (mod m) を使い、m を φ(m) に置き換えて再帰する。m は O(log m) 回で 1 になる
 * 途中の値は「本当の値が m 未満ならそのまま、m 以上なら (値 mod m) + m」で持ち、本当の値が m 以上かを失わないようにする
 * O(log m × (factorize(m) + log m))
 * @pre m <= 2 × 10^9
 */
inline long long tetration_mod(long long a, long long b, long long m) {
    // 途中の値は 2m 未満なので、その積があふれないように m を制限する
    assert(a >= 0 && b >= 0 && 1 <= m && m <= 2000000000);
    // x を上の形に直す
    auto clamp = [](long long x, long long mod) { return x < mod ? x : x % mod + mod; };
    // a↑↑b を上の形で返す
    auto rec = [&](auto self, long long b, long long mod) -> long long {
        if (b == 0) return clamp(1, mod);
        // 0↑↑b は b が偶数なら 1、奇数なら 0
        if (a == 0) return clamp(b % 2 == 0 ? 1 : 0, mod);
        // a >= 1 なら a↑↑b >= 1
        if (a == 1 || mod == 1) return clamp(1, mod);
        const long long e = self(self, b - 1, totient(mod));
        // a^e を上の形で求める (a >= 1、e >= 1)
        long long res = 1, base = clamp(a, mod);
        for (long long k = e; k > 0; k >>= 1) {
            if (k & 1) res = clamp(res * base, mod);
            if (k > 1) base = clamp(base * base, mod);
        }
        return res;
    };
    return rec(rec, b, m) % m;
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
