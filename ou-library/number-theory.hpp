#pragma once

#include <numeric>
#include <vector>
#include "modint.hpp"

/**
 * @brief a^(-1) mod MODを返す
 * @param a long long
 * @param MOD long long
 * @return long long
 */
long long modinv(long long a, long long MOD) {
    long long b = MOD, u = 1, v = 0;
    while (b) {
        long long t = a / b;
        a -= t * b; std::swap(a, b);
        u -= t * v; std::swap(u, v);
    }
    u %= MOD; 
    if (u < 0) u += MOD;
    return u;
}

/**
 * @brief a^n mod MODを返す
 * @param a long long
 * @param n long long
 * @param MOD long long
 * @return long long
 */
long long modpow(long long a, long long n, long long MOD) {
    long long res = 1;
    a %= MOD;
    if(n < 0) {
        n = -n;
        a = modinv(a, MOD);
    }
    while (n > 0) {
        if (n & 1) res = res * a % MOD;
        a = a * a % MOD;
        n >>= 1;
    }
    return res;
}

/**
 * @brief 2式の連立合同式を、mが互いに素になるように変形する
 * @param r1 long long
 * @param m1 long long
 * @param r2 long long
 * @param m2 long long
 * @note 矛盾する場合、r1 = r2 = m1 = m2 = -1となる
 */
void coprimize_simulaneous_congruence_equation(long long& r1, long long& m1, long long& r2, long long& m2) {
    long long g = std::gcd(m1, m2);
    if((r2 - r1) % g != 0) {
        r1 = r2 = m1 = m2 = -1;
        return;
    }
    m1 /= g, m2 /= g;
    long long gi = std::gcd(g, m1);
    long long gj = g / gi;
    do {
        g = std::gcd(gi, gj);
        gi *= g, gj /= g;
    } while(g != 1);
    m1 *= gi, m2 *= gj;
    r1 %= m1, r2 %= m2;
}

/**
 * @brief 連立合同式を解く
 * @param r vector<long long> 余りの配列
 * @param m vector<long long> modの配列
 * @return std::pair<long long, long long> (解, LCM) 解なしのときは{-1, -1}
 */
std::pair<long long, long long> crt(const std::vector<long long>& r, const std::vector<long long>& m) {
    assert(r.size() == m.size());
    if(r.size() == 0) return {0, 1};
    int n = (int)r.size();
    long long m_lcm = m[0];
    long long ans = r[0] % m[0];
    for (int i = 1; i < n; i++) {
        long long rr = r[i] % m[i], mm = m[i];
        coprimize_simulaneous_congruence_equation(ans, m_lcm, rr, mm);
        if(m_lcm == -1) return {-1, -1};
        long long t = ((rr - ans) * modinv(m_lcm, mm)) % mm;
        if(t < 0) t += mm;
        ans += t * m_lcm;
        m_lcm *= mm;
    }
    return {ans, m_lcm};
}

/**
 * @brief 連立合同式の最小の非負整数解 % MODを求める
 * @param r vector<long long> 余りの配列
 * @param m vector<long long> modの配列
 * @param MOD long long
 * @return std::pair<long long, long long> (最小解 % MOD, LCM % MOD) 解なしのときは{-1, -1}
 */
std::pair<long long, long long> crt(const std::vector<long long>& r, const std::vector<long long>& m, long long MOD) {
    assert(r.size() == m.size());
    if(r.size() == 0) return {0, 1};
    int n = (int)r.size();
    std::vector<long long> r2 = r, m2 = m;
    // mを互いに素にする
    for(int i = 1; i < n; i++) {
        for(int j = 0; j < i; j++) {
            coprimize_simulaneous_congruence_equation(r2[i], m2[i], r2[j], m2[j]);
            if(m2[i] == -1) return {-1, -1};
        }
    }

    m2.push_back(MOD);
    std::vector<long long> prod(n+1, 1); // m2[0] * ... * m2[i - 1] mod m2[i]
    std::vector<long long> x(n+1, 0); // i番目までの解 mod m2[i]
    for(int i = 0; i < n; i++) {
        long long t = (r2[i] - x[i]) * modinv(prod[i], m2[i]) % m2[i];
        if(t < 0) t += m2[i];
        for(int j = i + 1; j <= n; j++) {
            (x[j] += t * prod[j]) %= m2[j];
            (prod[j] *= m2[i]) %= m2[j];
        }
    }
    return {x[n], prod[n]};
}

/**
 * @brief 畳み込み
 */
namespace NTT {
    /**
     * @brief 原子根
     * @param MOD int
     * @return int
     */
    int calc_primitive_root(int MOD) {
        if (MOD == 2) return 1;
        if (MOD == 167772161) return 3;
        if (MOD == 469762049) return 3;
        if (MOD == 754974721) return 11;
        if (MOD == 998244353) return 3;
        int divs[20] = {};
        divs[0] = 2;
        int cnt = 1;
        long long x = (MOD - 1) >> 1;
        while (x % 2 == 0) x >>= 1;
        for (long long i = 3; i * i <= x; i += 2) {
            if (x % i == 0) {
                divs[cnt ++] = i;
                while (x % i == 0) x /= i;
            }
        }
        if (x > 1) divs[cnt++] = x;
        for (int g = 2;; ++ g) {
            bool ok = true;
            for (int i = 0; i < cnt; i++) {
                if (modpow(g, (MOD - 1) / divs[i], MOD) == 1) {
                    ok = false;
                    break;
                }
            }
            if (ok) return g;
        }
    }

    /**
     * @brief 畳み込みのサイズを2のべき乗にする
     */
    int get_fft_size(int N, int M) {
        int size_a = 1, size_b = 1;
        while (size_a < N) size_a <<= 1;
        while (size_b < M) size_b <<= 1;
        return std::max(size_a, size_b) << 1;
    }

    /**
     * @brief NTT
     */
    template<class mint> void trans(std::vector<mint>& v, bool inv = false) {
        if (v.empty()) return;
        int N = (int) v.size();
        int MOD = v[0].mod();
        int PR = calc_primitive_root(MOD);
        static bool first = true;
        static std::vector<long long> vbw(30), vibw(30);
        if (first) {
            first = false;
            for (int k = 0; k < 30; ++ k) {
                vbw[k] = modpow(PR, (MOD - 1) >> (k + 1), MOD);
                vibw[k] = modinv(vbw[k], MOD);
            }
        }
        for (int i = 0, j = 1; j < N - 1; ++ j) {
            for (int k = N >> 1; k > (i ^= k); k >>= 1);
            if (i > j) std::swap(v[i], v[j]);
        }
        for (int k = 0, t = 2; t <= N; ++ k, t <<= 1) {
            long long bw = vbw[k];
            if (inv) bw = vibw[k];
            for (int i = 0; i < N; i += t) {
                mint w = 1;
                for (int j = 0; j < (t >> 1); ++ j) {
                    int j1 = i + j, j2 = i + j + (t >> 1);
                    mint c1 = v[j1], c2 = v[j2] * w;
                    v[j1] = c1 + c2;
                    v[j2] = c1 - c2;
                    w *= bw;
                }
            }
        }
        if (inv) {
            long long invN = modinv(N, MOD);
            for (int i = 0; i < N; ++ i) v[i] = v[i] * invN;
        }
    }

    static constexpr int MOD0 = 754974721;
    static constexpr int MOD1 = 167772161;
    static constexpr int MOD2 = 469762049;
    using mint0 = static_modint<MOD0>;
    using mint1 = static_modint<MOD1>;
    using mint2 = static_modint<MOD2>;
    static const mint1 imod0 = 95869806; // modinv(MOD0, MOD1);
    static const mint2 imod1 = 104391568; // modinv(MOD1, MOD2);
    static const mint2 imod01 = 187290749; // imod1 / MOD0;

    /**
     * @brief 配列のサイズが小さいときの畳み込み
     * @param T mint, long long
     * @param A vector<T>
     * @param B vector<T>
     * @return vector<T>
     */
    template<class T> std::vector<T> naive
    (const std::vector<T>& A, const std::vector<T>& B) {
        if (A.empty() || B.empty()) return {};
        int N = (int) A.size(), M = (int) B.size();
        std::vector<T> res(N + M - 1);
        for (int i = 0; i < N; ++ i)
            for (int j = 0; j < M; ++ j)
                res[i + j] += A[i] * B[j];
        return res;
    }
};

/**
 * @brief modintの畳み込み
 * @param A vector<mint>
 * @param B vector<mint>
 * @return vector<mint>
 */
template<class mint> std::vector<mint> convolution
(const std::vector<mint>& A, const std::vector<mint>& B) {
    if (A.empty() || B.empty()) return {};
    int N = (int) A.size(), M = (int) B.size();
    if (std::min(N, M) < 30) return NTT::naive(A, B);
    int MOD = A[0].mod();
    int size_fft = NTT::get_fft_size(N, M);
    if (MOD == 998244353) {
        std::vector<mint> a(size_fft), b(size_fft), c(size_fft);
        for (int i = 0; i < N; ++i) a[i] = A[i];
        for (int i = 0; i < M; ++i) b[i] = B[i];
        NTT::trans(a), NTT::trans(b);
        std::vector<mint> res(size_fft);
        for (int i = 0; i < size_fft; ++i) res[i] = a[i] * b[i];
        NTT::trans(res, true);
        res.resize(N + M - 1);
        return res;
    }
    std::vector<NTT::mint0> a0(size_fft, 0), b0(size_fft, 0), c0(size_fft, 0);
    std::vector<NTT::mint1> a1(size_fft, 0), b1(size_fft, 0), c1(size_fft, 0);
    std::vector<NTT::mint2> a2(size_fft, 0), b2(size_fft, 0), c2(size_fft, 0);
    for (int i = 0; i < N; ++ i) {
        a0[i] = A[i].value();
        a1[i] = A[i].value();
        a2[i] = A[i].value();
    }
    for (int i = 0; i < M; ++ i) {
        b0[i] = B[i].value();
        b1[i] = B[i].value();
        b2[i] = B[i].value();
    }
    NTT::trans(a0), NTT::trans(a1), NTT::trans(a2), 
    NTT::trans(b0), NTT::trans(b1), NTT::trans(b2);
    for (int i = 0; i < size_fft; ++i) {
        c0[i] = a0[i] * b0[i];
        c1[i] = a1[i] * b1[i];
        c2[i] = a2[i] * b2[i];
    }
    NTT::trans(c0, true), NTT::trans(c1, true), NTT::trans(c2, true);
    static const mint mod0 = NTT::MOD0, mod01 = mod0 * NTT::MOD1;
    std::vector<mint> res(N + M - 1);
    for (int i = 0; i < N + M - 1; ++ i) {
        int y0 = c0[i].value();
        int y1 = (NTT::imod0 * (c1[i] - y0)).value();
        int y2 = (NTT::imod01 * (c2[i] - y0) - NTT::imod1 * y1).value();
        res[i] = mod01 * y2 + mod0 * y1 + y0;
    }
    return res;
}

/**
 * @brief long longの畳み込み
 * @param A vector<long long>
 * @param B vector<long long>
 * @return vector<long long>
 */
std::vector<long long> convolution_ll
(const std::vector<long long>& A, const std::vector<long long>& B) {
    if (A.empty() || B.empty()) return {};
    int N = (int) A.size(), M = (int) B.size();
    if (std::min(N, M) < 30) return NTT::naive(A, B);
    int size_fft = NTT::get_fft_size(N, M);
    std::vector<NTT::mint0> a0(size_fft, 0), b0(size_fft, 0), c0(size_fft, 0);
    std::vector<NTT::mint1> a1(size_fft, 0), b1(size_fft, 0), c1(size_fft, 0);
    std::vector<NTT::mint2> a2(size_fft, 0), b2(size_fft, 0), c2(size_fft, 0);
    for (int i = 0; i < N; ++ i) {
        a0[i] = A[i];
        a1[i] = A[i];
        a2[i] = A[i];
    }
    for (int i = 0; i < M; ++ i) {
        b0[i] = B[i];
        b1[i] = B[i];
        b2[i] = B[i];
    }
    NTT::trans(a0), NTT::trans(a1), NTT::trans(a2), 
    NTT::trans(b0), NTT::trans(b1), NTT::trans(b2);
    for (int i = 0; i < size_fft; ++ i) {
        c0[i] = a0[i] * b0[i];
        c1[i] = a1[i] * b1[i];
        c2[i] = a2[i] * b2[i];
    }
    NTT::trans(c0, true), NTT::trans(c1, true), NTT::trans(c2, true);
    static const long long mod0 = NTT::MOD0, mod01 = mod0 * NTT::MOD1;
    static const __int128_t mod012 = (__int128_t)mod01 * NTT::MOD2;
    std::vector<long long> res(N + M - 1);
    for (int i = 0; i < N + M - 1; ++ i) {
        int y0 = c0[i].value();
        int y1 = (NTT::imod0 * (c1[i] - y0)).value();
        int y2 = (NTT::imod01 * (c2[i] - y0) - NTT::imod1 * y1).value();
        __int128_t tmp = (__int128_t)mod01 * y2 + (__int128_t)mod0 * y1 + y0;
        if(tmp < (mod012 >> 1)) res[i] = tmp;
        else res[i] = tmp - mod012;
    }
    return res;
}
