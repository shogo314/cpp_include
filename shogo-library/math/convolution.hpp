#pragma once

/**
 * @file convolution.hpp
 * @brief 畳み込み (NTT)
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "modint.hpp"
#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/convolution.hpp

/**
 * @brief 畳み込み
 */
namespace internal_convolution {
/**
 * @brief 原子根
 * @param MOD int
 * @return int
 */
inline int calc_primitive_root(int MOD) {
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
            divs[cnt++] = i;
            while (x % i == 0) x /= i;
        }
    }
    if (x > 1) divs[cnt++] = x;
    for (int g = 2;; ++g) {
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
inline int get_fft_size(int N, int M) {
    int size = 1;
    while (size < N + M - 1) size <<= 1;
    return size;
}

/**
 * @brief x の末尾の 0 の個数 (x > 0)
 */
constexpr int countr_zero(unsigned int x) {
    int k = 0;
    while (!(x & 1)) x >>= 1, k++;
    return k;
}

template <class T>
struct is_static_modint : std::false_type {};
template <std::uint32_t Mod>
struct is_static_modint<static_modint<Mod>> : std::true_type {};

/**
 * @brief NTT の回転因子の表 (AtCoder Library と同じ作り)
 *
 * root[i] は 1 の原始 2^i 乗根、rate2・rate3 は基数 2・4 のバタフライで次のブロックに進むときに掛ける値
 */
template <class mint>
struct FFTInfo {
    static constexpr int rank2 = countr_zero(mint::mod() - 1);
    std::array<mint, rank2 + 1> root, iroot;
    std::array<mint, std::max(0, rank2 - 1)> rate2, irate2;
    std::array<mint, std::max(0, rank2 - 2)> rate3, irate3;

    FFTInfo() {
        const mint g = calc_primitive_root(mint::mod());
        root[rank2] = g.pow((mint::mod() - 1) >> rank2);
        iroot[rank2] = root[rank2].inv();
        for (int i = rank2 - 1; i >= 0; i--) {
            root[i] = root[i + 1] * root[i + 1];
            iroot[i] = iroot[i + 1] * iroot[i + 1];
        }
        mint prod = 1, iprod = 1;
        for (int i = 0; i <= rank2 - 2; i++) {
            rate2[i] = root[i + 2] * prod;
            irate2[i] = iroot[i + 2] * iprod;
            prod *= iroot[i + 2];
            iprod *= root[i + 2];
        }
        prod = 1, iprod = 1;
        for (int i = 0; i <= rank2 - 3; i++) {
            rate3[i] = root[i + 3] * prod;
            irate3[i] = iroot[i + 3] * iprod;
            prod *= iroot[i + 3];
            iprod *= root[i + 3];
        }
    }
};

/**
 * @brief NTT (ビット反転の並べ替えをしない。結果はビット反転した順に並ぶ)。基数 4 のバタフライを使う
 * a の長さは 2 のべき乗で、2^rank2 以下であること
 */
template <class mint>
void butterfly(std::vector<mint>& a) {
    const int n = static_cast<int>(a.size());
    const int h = countr_zero(n);
    static const FFTInfo<mint> info;
    int len = 0;  // a[i, i + (n >> len), i + 2 * (n >> len), ...] は変換済み
    while (len < h) {
        if (h - len == 1) {
            int p = 1 << (h - len - 1);
            mint rot = 1;
            for (int s = 0; s < (1 << len); s++) {
                int offset = s << (h - len);
                for (int i = 0; i < p; i++) {
                    mint l = a[i + offset], r = a[i + offset + p] * rot;
                    a[i + offset] = l + r;
                    a[i + offset + p] = l - r;
                }
                if (s + 1 != (1 << len)) rot *= info.rate2[countr_zero(~static_cast<unsigned int>(s))];
            }
            len++;
        } else {
            int p = 1 << (h - len - 2);
            mint rot = 1, imag = info.root[2];
            const unsigned long long mod2 = 1ULL * mint::mod() * mint::mod();
            for (int s = 0; s < (1 << len); s++) {
                mint rot2 = rot * rot, rot3 = rot2 * rot;
                int offset = s << (h - len);
                for (int i = 0; i < p; i++) {
                    unsigned long long a0 = a[i + offset].val();
                    unsigned long long a1 = 1ULL * a[i + offset + p].val() * rot.val();
                    unsigned long long a2 = 1ULL * a[i + offset + 2 * p].val() * rot2.val();
                    unsigned long long a3 = 1ULL * a[i + offset + 3 * p].val() * rot3.val();
                    unsigned long long a1na3imag = 1ULL * mint(a1 + mod2 - a3).val() * imag.val();
                    unsigned long long na2 = mod2 - a2;
                    a[i + offset] = mint(a0 + a2 + a1 + a3);
                    a[i + offset + p] = mint(a0 + a2 + (2 * mod2 - (a1 + a3)));
                    a[i + offset + 2 * p] = mint(a0 + na2 + a1na3imag);
                    a[i + offset + 3 * p] = mint(a0 + na2 + (mod2 - a1na3imag));
                }
                if (s + 1 != (1 << len)) rot *= info.rate3[countr_zero(~static_cast<unsigned int>(s))];
            }
            len += 2;
        }
    }
}

/**
 * @brief butterfly の逆変換 (n で割らない)
 */
template <class mint>
void butterfly_inv(std::vector<mint>& a) {
    const int n = static_cast<int>(a.size());
    const int h = countr_zero(n);
    static const FFTInfo<mint> info;
    int len = h;
    while (len) {
        if (len == 1) {
            int p = 1 << (h - len);
            mint irot = 1;
            for (int s = 0; s < (1 << (len - 1)); s++) {
                int offset = s << (h - len + 1);
                for (int i = 0; i < p; i++) {
                    mint l = a[i + offset], r = a[i + offset + p];
                    a[i + offset] = l + r;
                    a[i + offset + p] = mint(1ULL * (mint::mod() + l.val() - r.val()) * irot.val());
                }
                if (s + 1 != (1 << (len - 1))) irot *= info.irate2[countr_zero(~static_cast<unsigned int>(s))];
            }
            len--;
        } else {
            int p = 1 << (h - len);
            mint irot = 1, iimag = info.iroot[2];
            for (int s = 0; s < (1 << (len - 2)); s++) {
                mint irot2 = irot * irot, irot3 = irot2 * irot;
                int offset = s << (h - len + 2);
                for (int i = 0; i < p; i++) {
                    unsigned long long a0 = a[i + offset].val();
                    unsigned long long a1 = a[i + offset + p].val();
                    unsigned long long a2 = a[i + offset + 2 * p].val();
                    unsigned long long a3 = a[i + offset + 3 * p].val();
                    unsigned long long a2na3iimag = mint((mint::mod() + a2 - a3) * iimag.val()).val();
                    a[i + offset] = mint(a0 + a1 + a2 + a3);
                    a[i + offset + p] = mint((a0 + (mint::mod() - a1) + a2na3iimag) * irot.val());
                    a[i + offset + 2 * p] = mint((a0 + a1 + (mint::mod() - a2) + (mint::mod() - a3)) * irot2.val());
                    a[i + offset + 3 * p] = mint((a0 + (mint::mod() - a1) + (mint::mod() - a2na3iimag)) * irot3.val());
                }
                if (s + 1 != (1 << (len - 2))) irot *= info.irate3[countr_zero(~static_cast<unsigned int>(s))];
            }
            len -= 2;
        }
    }
}

/**
 * @brief NTT で畳み込む (法は素数で、長さ N + M - 1 以上の 2 のべき乗が 2^rank2 以下であること)
 * same なら B は A と同じとみなし、変換を 1 回省く
 */
template <class mint>
std::vector<mint> convolution_fft(std::vector<mint> a, std::vector<mint> b, bool same = false) {
    const int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    const int z = get_fft_size(n, m);
    a.resize(z);
    butterfly(a);
    if (same) {
        for (int i = 0; i < z; i++) a[i] *= a[i];
    } else {
        b.resize(z);
        butterfly(b);
        for (int i = 0; i < z; i++) a[i] *= b[i];
    }
    butterfly_inv(a);
    a.resize(n + m - 1);
    const mint iz = mint(z).inv();
    for (int i = 0; i < n + m - 1; i++) a[i] *= iz;
    return a;
}

/**
 * @brief mint で直接 NTT できるか (static_modint で、法が素数で、2^rank2 が長さ z 以上)
 */
template <class mint>
constexpr bool ntt_friendly(int z) {
    if constexpr (is_static_modint<mint>::value) {
        return internal_modint::is_prime32(mint::mod()) && mint::mod() > 2 && (mint::mod() - 1) % z == 0;
    } else {
        return false;
    }
}

/**
 * @brief 長さ 61 以上の畳み込みを mint で直接 NTT できる見込みがあるか (コンパイル時に決まる部分だけ)
 * 偽なら NTT の関数を実体化しない (法 - 1 の 2 の指数が小さいと、回転因子の表の外を読むという誤った警告が出るため)
 */
template <class mint>
constexpr bool ntt_candidate() {
    if constexpr (is_static_modint<mint>::value) {
        return countr_zero(mint::mod() - 1) >= 7;
    } else {
        return false;
    }
}

static constexpr int MOD0 = 754974721;
static constexpr int MOD1 = 167772161;
static constexpr int MOD2 = 469762049;
using mint0 = static_modint<MOD0>;
using mint1 = static_modint<MOD1>;
using mint2 = static_modint<MOD2>;
static const mint1 imod0 = 95869806;    // modinv(MOD0, MOD1);
static const mint2 imod1 = 104391568;   // modinv(MOD1, MOD2);
static const mint2 imod01 = 187290749;  // imod1 / MOD0;

/**
 * @brief 配列のサイズが小さいときの畳み込み
 * @param T mint, long long
 * @param A vector<T>
 * @param B vector<T>
 * @return vector<T>
 */
template <class T>
std::vector<T> naive(const std::vector<T>& A, const std::vector<T>& B) {
    if (A.empty() || B.empty()) return {};
    int N = static_cast<int>(A.size()), M = static_cast<int>(B.size());
    std::vector<T> res(N + M - 1);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < M; ++j)
            res[i + j] += A[i] * B[j];
    return res;
}
};  // namespace internal_convolution

namespace internal_convolution {
/**
 * @brief 3 つの法で畳み込み、中国剰余定理で値 (0 以上 MOD0 MOD1 MOD2 未満) を復元する
 * 値は y0 + y1 MOD0 + y2 MOD0 MOD1 (0 <= y0 < MOD0、0 <= y1 < MOD1、0 <= y2 < MOD2) の形になり、各項を f(y0, y1, y2) で組み立てる
 * A と B が同じ配列なら、変換を 1 回省く
 */
template <class T, class R, class F>
std::vector<R> convolution_crt(const std::vector<T>& A, const std::vector<T>& B, F f) {
    const int N = static_cast<int>(A.size()), M = static_cast<int>(B.size());
    std::vector<mint0> a0(N), b0(M);
    std::vector<mint1> a1(N), b1(M);
    std::vector<mint2> a2(N), b2(M);
    for (int i = 0; i < N; ++i) a0[i] = A[i], a1[i] = A[i], a2[i] = A[i];
    for (int i = 0; i < M; ++i) b0[i] = B[i], b1[i] = B[i], b2[i] = B[i];
    const bool same = &A == &B;
    auto c0 = convolution_fft(std::move(a0), std::move(b0), same);
    auto c1 = convolution_fft(std::move(a1), std::move(b1), same);
    auto c2 = convolution_fft(std::move(a2), std::move(b2), same);
    std::vector<R> res(N + M - 1);
    for (int i = 0; i < N + M - 1; ++i) {
        unsigned int y0 = c0[i].val();
        unsigned int y1 = (imod0 * (c1[i] - y0)).val();
        unsigned int y2 = (imod01 * (c2[i] - y0) - imod1 * y1).val();
        res[i] = f(y0, y1, y2);
    }
    return res;
}

/**
 * @brief convolution_u64 の 1 つの法での計算。A = A0 + A1 2^32、B = B0 + B1 2^32 と分け、
 * A0 B0 と A0 B1 + A1 B0 の畳み込みを、4 回の変換と 2 回の逆変換で求める
 */
template <class mint>
std::array<std::vector<mint>, 2> convolution_u64_parts(const std::vector<unsigned long long>& A, const std::vector<unsigned long long>& B, int z) {
    const int N = static_cast<int>(A.size()), M = static_cast<int>(B.size());
    std::vector<mint> a0(z), a1(z), b0(z), b1(z);
    for (int i = 0; i < N; ++i) a0[i] = A[i] & 0xffffffffULL, a1[i] = A[i] >> 32;
    for (int i = 0; i < M; ++i) b0[i] = B[i] & 0xffffffffULL, b1[i] = B[i] >> 32;
    butterfly(a0), butterfly(a1), butterfly(b0), butterfly(b1);
    for (int i = 0; i < z; ++i) {
        const mint x0 = a0[i], x1 = a1[i];
        a0[i] = x0 * b0[i];
        a1[i] = x0 * b1[i] + x1 * b0[i];
    }
    butterfly_inv(a0), butterfly_inv(a1);
    a0.resize(N + M - 1), a1.resize(N + M - 1);
    const mint iz = mint(z).inv();
    for (int i = 0; i < N + M - 1; ++i) a0[i] *= iz, a1[i] *= iz;
    return {a0, a1};
}
}  // namespace internal_convolution

/**
 * @brief modintの畳み込み
 *
 * static_modint で法が NTT に向く素数 (998244353 など) なら、その法で直接 NTT する。
 * そうでなければ 3 つの法で畳み込み、中国剰余定理で復元する
 * @param A vector<mint>
 * @param B vector<mint>
 * @return vector<mint>
 */
template <class mint>
std::vector<mint> convolution(const std::vector<mint>& A, const std::vector<mint>& B) {
    if (A.empty() || B.empty()) return {};
    const int N = static_cast<int>(A.size()), M = static_cast<int>(B.size());
    if (std::min(N, M) <= 60) return internal_convolution::naive(A, B);
    const int z = internal_convolution::get_fft_size(N, M);
    if constexpr (internal_convolution::ntt_candidate<mint>()) {
        if (internal_convolution::ntt_friendly<mint>(z)) return internal_convolution::convolution_fft(A, B, &A == &B);
    }
    std::vector<unsigned int> a(N), b(M);
    for (int i = 0; i < N; ++i) a[i] = A[i].val();
    for (int i = 0; i < M; ++i) b[i] = B[i].val();
    const mint mod0 = internal_convolution::MOD0, mod01 = mod0 * internal_convolution::MOD1;
    return internal_convolution::convolution_crt<unsigned int, mint>(a, &A == &B ? a : b, [&](unsigned int y0, unsigned int y1, unsigned int y2) {
        return mod01 * y2 + mod0 * y1 + y0;
    });
}

/**
 * @brief long longの畳み込み
 * 結果の各項の絶対値が MOD0 MOD1 MOD2 / 2 (約 3 * 10^25) 未満で、long long に収まること
 * @param A vector<long long>
 * @param B vector<long long>
 * @return vector<long long>
 */
inline std::vector<long long> convolution_ll(const std::vector<long long>& A, const std::vector<long long>& B) {
    if (A.empty() || B.empty()) return {};
    const int N = static_cast<int>(A.size()), M = static_cast<int>(B.size());
    if (std::min(N, M) <= 60) return internal_convolution::naive(A, B);
    static const long long mod0 = internal_convolution::MOD0, mod01 = mod0 * internal_convolution::MOD1;
    static const __int128_t mod012 = static_cast<__int128_t>(mod01) * internal_convolution::MOD2;
    return internal_convolution::convolution_crt<long long, long long>(A, B, [&](unsigned int y0, unsigned int y1, unsigned int y2) {
        __int128_t tmp = static_cast<__int128_t>(mod01) * y2 + static_cast<__int128_t>(mod0) * y1 + y0;
        return static_cast<long long>(tmp < (mod012 >> 1) ? tmp : tmp - mod012);
    });
}

/**
 * @brief unsigned long long の畳み込みを 2^64 で割った余りで求める
 * 各値を 32 ビットずつ 2 つに分け、下位どうしの積の和と、上位と下位の積の和を 3 つの法と中国剰余定理で求める。
 * 和の項の数 min(|A|, |B|) が 10^6 以下であること (和が MOD0 MOD1 MOD2 未満に収まる)
 * @param A vector<unsigned long long>
 * @param B vector<unsigned long long>
 * @return vector<unsigned long long>
 */
inline std::vector<unsigned long long> convolution_u64(const std::vector<unsigned long long>& A, const std::vector<unsigned long long>& B) {
    if (A.empty() || B.empty()) return {};
    const int N = static_cast<int>(A.size()), M = static_cast<int>(B.size());
    if (std::min(N, M) <= 60) return internal_convolution::naive(A, B);
    assert(std::min(N, M) <= 1000000);
    using namespace internal_convolution;
    const int z = get_fft_size(N, M);
    const auto [p0, q0] = convolution_u64_parts<mint0>(A, B, z);
    const auto [p1, q1] = convolution_u64_parts<mint1>(A, B, z);
    const auto [p2, q2] = convolution_u64_parts<mint2>(A, B, z);
    // 値は y0 + y1 MOD0 + y2 MOD0 MOD1 なので、unsigned long long で計算すれば 2^64 で割った余りになる
    constexpr unsigned long long mod01 = static_cast<unsigned long long>(MOD0) * MOD1;
    auto restore = [&](mint0 c0, mint1 c1, mint2 c2) {
        const unsigned int y0 = c0.val();
        const unsigned int y1 = (imod0 * (c1 - y0)).val();
        const unsigned int y2 = (imod01 * (c2 - y0) - imod1 * y1).val();
        return y0 + static_cast<unsigned long long>(y1) * MOD0 + y2 * mod01;
    };
    std::vector<unsigned long long> res(N + M - 1);
    for (int i = 0; i < N + M - 1; ++i) res[i] = restore(p0[i], p1[i], p2[i]) + (restore(q0[i], q1[i], q2[i]) << 32);
    return res;
}
