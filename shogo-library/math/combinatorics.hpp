#pragma once

/**
 * @file combinatorics.hpp
 * @brief 組み合わせ
 *
 * 素数 mod での階乗・二項係数は Combination、素数冪 p^q を法とする二項係数は CombinationPQ
 */

#include <cassert>
#include <numeric>
#include <vector>

#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/math/combinatorics.hpp

/**
 * @brief 素数 mod での階乗・二項係数など
 *
 * 例:
 * using C = Combination<static_modint<998244353>>;
 * C::C(5, 2);  // 10
 *
 * static メンバ関数だけのクラス。階乗とその逆元を必要になった分だけ前計算し、同じ Modint の呼び出しで使い回す
 * 前計算を使う関数 (factorial・C など) は、n が mod 未満であること (n! が 0 になり逆元がない)。
 * n が mod 以上なら、r が小さいときは C_loop、mod が小さいときは C_lucas を使う
 *
 * @tparam Modint 法が素数の modint (dynamic_modint で法を変えたら前計算をやり直す)
 */
template <typename Modint>
class Combination {
    static std::vector<Modint> fact, inv_fact;
    static long long cached_mod;  // dynamic_modint で mod が変わったら前計算をやり直す

   public:
    /**
     * @brief n までの階乗とその逆元を前計算する
     * @param n 前計算する上限 (mod 未満)
     * @note O(n)。各関数が必要な分だけ自動で呼ぶ。小さい n から少しずつ伸びると逆元の計算 (O(log mod)) が毎回入るので、
     * 使う最大の n が分かっていれば最初に呼んでおくとよい
     */
    inline static void extend(long long n) {
        if (cached_mod != static_cast<long long>(Modint::mod())) {
            cached_mod = Modint::mod();
            fact = {1, 1};
            inv_fact = {1, 1};
        }
        const long long m = fact.size();
        if (n < m) return;
        assert(n < static_cast<long long>(Modint::mod()) && "Combination: n must be less than mod (use C_loop or C_lucas)");
        fact.resize(n + 1);
        inv_fact.resize(n + 1);
        for (long long i = m; i <= n; ++i) {
            fact[i] = fact[i - 1] * i;
        }
        inv_fact[n] = fact[n].inv();
        for (long long i = n; i > m; --i) {
            inv_fact[i - 1] = inv_fact[i] * i;
        }
    }

    /**
     * @brief n の階乗
     * @param n 0 以上 mod 未満
     * @return n!
     * @note 初回は extend(n) の O(n)、以降 O(1)
     */
    inline static Modint factorial(long long n) {
        assert(n >= 0);
        extend(n);
        return fact[n];
    }
    /**
     * @brief n の階乗の逆元
     * @param n 0 以上 mod 未満
     * @return (n!)^{-1}
     * @note 初回は extend(n) の O(n)、以降 O(1)
     */
    inline static Modint inverse_factorial(long long n) {
        assert(n >= 0);
        extend(n);
        return inv_fact[n];
    }
    /**
     * @brief n の逆元 (階乗の前計算を使う。1 つだけなら Modint(n).inv() でよい)
     * @param n 1 以上 mod 未満
     * @return n^{-1}
     * @note 初回は extend(n) の O(n)、以降 O(1)
     */
    inline static Modint inv(long long n) {
        assert(n > 0);
        extend(n);
        return inv_fact[n] * fact[n - 1];
    }

    /**
     * @brief 順列の数 nPr = n! / (n - r)! (n 個から r 個を選んで並べる方法の数)
     * @param n mod 未満
     * @param r
     * @return nPr。r < 0 か n < r なら 0
     * @note 初回は extend(n) の O(n)、以降 O(1)
     */
    inline static Modint P(long long n, long long r) {
        if (r < 0 || n < r) return 0;
        extend(n);
        return fact[n] * inv_fact[n - r];
    }
    /**
     * @brief 二項係数 nCr = n! / (r! (n - r)!) (n 個から r 個を選ぶ方法の数)
     * @param n mod 未満
     * @param r
     * @return nCr。r < 0 か n < r なら 0
     * @note 初回は extend(n) の O(n)、以降 O(1)
     */
    inline static Modint C(long long n, long long r) {
        if (r < 0 || n < r) return 0;
        extend(n);
        return fact[n] * inv_fact[r] * inv_fact[n - r];
    }
    /**
     * @brief 重複組合せの数 nHr = (n+r-1)Cr (n 種類から重複を許して r 個を選ぶ方法の数)
     * @param n 0 以上 (n + r - 1 が mod 未満)
     * @param r 0 以上
     * @return nHr。n < 0 か r < 0 なら 0。n = 0 なら、r = 0 のとき 1 (何も選ばない 1 通り)、r > 0 のとき 0
     * @note 初回は extend(n + r - 1) の O(n + r)、以降 O(1)
     */
    inline static Modint H(long long n, long long r) {
        if (n < 0 || r < 0) return 0;
        if (n == 0 && r == 0) return 1;
        return C(n + r - 1, r);
    }

    /**
     * @brief nPr = n (n - 1) ... (n - r + 1) を定義どおり掛けて求める
     * @param n 大きくてもよい (mod 以上でもよい)
     * @param r
     * @return nPr。r < 0 か n < r なら 0
     * @note O(r)。前計算を使わない
     */
    inline static Modint P_loop(long long n, long long r) {
        if (r < 0 || n < r) return 0;
        Modint res = 1;
        for (long long i = 0; i < r; ++i) {
            res *= n - i;
        }
        return res;
    }
    /**
     * @brief nCr = nPr / r! を、nPr を定義どおり掛けて求める (n が 10^18 などでも r が小さければ使える)
     * @param n 大きくてもよい (mod 以上でもよい)
     * @param r min(r, n - r) が mod 未満
     * @return nCr。r < 0 か n < r なら 0
     * @note O(min(r, n - r))。r! の逆元に extend(min(r, n - r)) を使う
     */
    inline static Modint C_loop(long long n, long long r) {
        if (r < 0 || n < r) return 0;
        if (r > n - r) r = n - r;
        extend(r);
        return P_loop(n, r) * inv_fact[r];
    }
    /**
     * @brief nHr = (n+r-1)Cr を C_loop で求める
     * @param n 大きくてもよい
     * @param r
     * @return nHr。範囲外の扱いは H と同じ
     * @note O(min(r, n - 1))
     */
    inline static Modint H_loop(long long n, long long r) {
        if (n < 0 || r < 0) return 0;
        if (n == 0 && r == 0) return 1;
        return C_loop(n + r - 1, r);
    }

    /**
     * @brief nCr を Lucas の定理で求める (n、r を mod 進数で表し、桁ごとの二項係数の積にする)
     * @param n 大きくてもよい (mod 以上でもよい)
     * @param r
     * @return nCr。r < 0 か n < r なら 0
     * @note mod は小さい素数 (10^6 程度まで) を想定する。前計算に最大で extend(mod - 1) の O(mod)、以降 O(log_mod n)
     */
    inline static Modint C_lucas(long long n, long long r) {
        if (r < 0 || n < r) return 0;
        if (r == 0 || n == r) return 1;
        Modint res = 1;
        while (r > 0) {
            int ni = n % Modint::mod(), ri = r % Modint::mod();
            if (ni < ri) return 0;
            res *= C(ni, ri);
            n /= Modint::mod();
            r /= Modint::mod();
        }
        return res;
    }
};

template <typename Modint>
std::vector<Modint> Combination<Modint>::fact{1, 1};
template <typename Modint>
std::vector<Modint> Combination<Modint>::inv_fact{1, 1};
template <typename Modint>
long long Combination<Modint>::cached_mod = 0;

/**
 * @brief 素数冪 p^q を法とする二項係数 (n が大きくても、mod が素数でなくてもよい)
 *
 * 例:
 * CombinationPQ cpq(2, 3);  // mod 8
 * cpq.C(5, 2);              // 10 mod 8 = 2
 *
 * 素数でない mod M では、M = p1^q1 ... pk^qk と素因数分解し、各 p^q で C を求めて crt (number-theory.hpp) でまとめる
 * 参考: https://nyaannyaan.github.io/library/modulo/arbitrary-mod-binomial.hpp (一般化 Lucas の定理)
 */
struct CombinationPQ {
    int p;                                //!< 素数
    int q;                                //!< 指数
    int pq;                               //!< p^q (法)
    std::vector<int> fact_p, inv_fact_p;  // p の倍数を除いた階乗 (mod p^q) とその逆元
    int delta;                            // 符号の補正。p = 2 かつ q >= 3 なら 1、そうでなければ -1

    /**
     * @brief p^q 未満の数についての階乗を前計算する
     * @param p 素数
     * @param q 1 以上。p^q は int に収まり、配列 2 本を持てる大きさ (10^7 程度まで) であること
     * @note O(p^q)
     */
    CombinationPQ(int p, int q) : p(p), q(q) {
        assert(p >= 2 && q >= 1);
        pq = 1;
        for (int i = 0; i < q; i++) pq *= p;
        fact_p.resize(pq);
        fact_p[0] = 1;
        for (int i = 1; i < pq; i++) {
            if (i % p == 0)
                fact_p[i] = fact_p[i - 1];
            else
                fact_p[i] = static_cast<long long>(fact_p[i - 1]) * i % pq;
        }
        inv_fact_p.resize(pq);
        inv_fact_p[pq - 1] = inv_mod(fact_p[pq - 1], pq);
        for (int i = pq - 1; i > 0; i--) {
            if (i % p == 0)
                inv_fact_p[i - 1] = inv_fact_p[i];
            else
                inv_fact_p[i - 1] = static_cast<long long>(inv_fact_p[i]) * i % pq;
        }
        if (p == 2 && q >= 3)
            delta = 1;
        else
            delta = -1;
    }
    /**
     * @brief nCr mod p^q
     * @param n 0 以上 (大きくてもよい)
     * @param r
     * @return nCr mod p^q を [0, p^q) の int で。r < 0 か n < r なら 0
     * @note O(log_p n)
     */
    int C(long long n, long long r) {
        if (r < 0 || n < r) return 0;
        long long m = n - r;
        int ans = 1;
        std::vector<int> epsilon;
        while (n > 0) {
            ans = static_cast<long long>(ans) * fact_p[n % pq] % pq;
            ans = static_cast<long long>(ans) * inv_fact_p[m % pq] % pq;
            ans = static_cast<long long>(ans) * inv_fact_p[r % pq] % pq;
            n /= p;
            m /= p;
            r /= p;
            epsilon.push_back(n - m - r);
        }
        if (delta == -1 && static_cast<int>(epsilon.size()) >= q && std::accumulate(epsilon.begin() + q - 1, epsilon.end(), 0) % 2 == 1) ans = pq - ans;
        if (ans == pq) ans = 0;
        int e = std::accumulate(epsilon.begin(), epsilon.end(), 0);
        if (e >= q)
            ans = 0;
        else {
            for (int i = 0; i < e; i++) {
                ans = static_cast<long long>(ans) * p % pq;
            }
        }
        return ans;
    }
};
