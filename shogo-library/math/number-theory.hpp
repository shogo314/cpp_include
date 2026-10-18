#pragma once

/**
 * @file number-theory.hpp
 * @brief 数論 (中国剰余定理)
 */

#include <cassert>
#include <numeric>
#include <optional>
#include <vector>

#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/math/number-theory.hpp

/**
 * @brief 2式の連立合同式を、mが互いに素になるように変形する
 * @param r1 long long
 * @param m1 long long
 * @param r2 long long
 * @param m2 long long
 * @return 矛盾しなければ true。矛盾する場合は false を返し、引数は変えない
 */
inline bool coprimize_simultaneous_congruence_equation(long long& r1, long long& m1, long long& r2, long long& m2) {
    long long g = std::gcd(m1, m2);
    if ((r2 - r1) % g != 0) return false;
    m1 /= g, m2 /= g;
    long long gi = std::gcd(g, m1);
    long long gj = g / gi;
    do {
        g = std::gcd(gi, gj);
        gi *= g, gj /= g;
    } while (g != 1);
    m1 *= gi, m2 *= gj;
    r1 %= m1, r2 %= m2;
    return true;
}

/**
 * @brief crt の戻り値
 *
 * 解は x + lcm * k (k は整数) と表せる
 */
struct CRTResult {
    long long x;    //!< 最小の非負整数解
    long long lcm;  //!< m の最小公倍数
};

/**
 * @brief 連立合同式を解く
 * @param r vector<long long> 余りの配列
 * @param m vector<long long> modの配列
 * @return std::optional<CRTResult> (解, LCM) 解なしのときは std::nullopt
 */
inline std::optional<CRTResult> crt(const std::vector<long long>& r, const std::vector<long long>& m) {
    assert(r.size() == m.size());
    if (r.size() == 0) return CRTResult{0, 1};
    int n = (int)r.size();
    long long m_lcm = m[0];
    long long ans = (r[0] % m[0] + m[0]) % m[0];
    for (int i = 1; i < n; i++) {
        long long rr = (r[i] % m[i] + m[i]) % m[i], mm = m[i];
        if (!coprimize_simultaneous_congruence_equation(ans, m_lcm, rr, mm)) return std::nullopt;
        long long t = ((rr - ans) * modinv(m_lcm, mm)) % mm;
        if (t < 0) t += mm;
        ans += t * m_lcm;
        m_lcm *= mm;
    }
    return CRTResult{ans, m_lcm};
}

/**
 * @brief 連立合同式の最小の非負整数解 % MODを求める
 * @param r vector<long long> 余りの配列
 * @param m vector<long long> modの配列
 * @param MOD long long
 * @return std::optional<CRTResult> (最小解 % MOD, LCM % MOD) 解なしのときは std::nullopt
 */
inline std::optional<CRTResult> crt(const std::vector<long long>& r, const std::vector<long long>& m, long long MOD) {
    assert(r.size() == m.size());
    if (r.size() == 0) return CRTResult{0, 1};
    int n = (int)r.size();
    std::vector<long long> r2 = r, m2 = m;
    // mを互いに素にする
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (!coprimize_simultaneous_congruence_equation(r2[i], m2[i], r2[j], m2[j])) return std::nullopt;
        }
    }

    m2.push_back(MOD);
    std::vector<long long> prod(n + 1, 1);  // m2[0] * ... * m2[i - 1] mod m2[i]
    std::vector<long long> x(n + 1, 0);     // i番目までの解 mod m2[i]
    for (int i = 0; i < n; i++) {
        long long t = (r2[i] - x[i]) * modinv(prod[i], m2[i]) % m2[i];
        if (t < 0) t += m2[i];
        for (int j = i + 1; j <= n; j++) {
            (x[j] += t * prod[j]) %= m2[j];
            (prod[j] *= m2[i]) %= m2[j];
        }
    }
    return CRTResult{x[n], prod[n]};
}
