#pragma once

/**
 * @file linear-recurrence.hpp
 * @brief 線形漸化式 (Bostan-Mori による第 k 項、Berlekamp-Massey による漸化式の復元)
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "convolution.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/linear-recurrence.hpp

namespace internal_linear_recurrence {

/**
 * @brief bostan_mori を NTT した値のまま進める版 (法が NTT に向く素数のとき)
 * 長さ 2N で変換すると、butterfly の結果はビット反転した順に並び、位置 2i と 2i+1 は点 w と -w での値になる。
 * そこで Q(-x) は隣どうしを入れ替えるだけで得られ、偶数次・奇数次の部分の値 (長さ N の変換) も
 * (F(w) + F(-w)) / 2、(F(w) - F(-w)) / (2w) で求まる。次の段の長さ 2N の変換は、
 * 残りの半分 (点 w ω_{2N} での値) だけを逆変換と変換 1 回ずつで作る。1 段あたりの NTT は長さ 2N を 2 回分ほどになる
 */
template <typename mint>
mint bostan_mori_ntt(std::vector<mint> P, std::vector<mint> Q, long long k, int h) {
    using namespace internal_convolution;
    static const FFTInfo<mint> info;
    const int N = 1 << h;
    // half_iw[i] = 1 / (2w) (w は長さ 2N の位置 2i の点 ω_{2N}^{rev[i]})、tw[i] = ω_{2N}^i / N
    const mint inv2 = mint(2).inv();
    std::vector<int> rev(N);
    for (int i = 1; i < N; i++) rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (h - 1));
    std::vector<mint> pw(N), half_iw(N), tw(N);
    pw[0] = 1, tw[0] = mint(N).inv();
    for (int i = 1; i < N; i++) {
        pw[i] = pw[i - 1] * info.iroot[h + 1];
        tw[i] = tw[i - 1] * info.root[h + 1];
    }
    for (int i = 0; i < N; i++) half_iw[i] = pw[rev[i]] * inv2;
    P.resize(2 * N), Q.resize(2 * N);
    butterfly(P), butterfly(Q);
    // 長さ N の変換 f から、長さ 2N の変換 (前半は f のまま) を作る
    auto extend = [&](std::vector<mint>& f) {
        std::vector<mint> g(f.begin(), f.begin() + N);
        butterfly_inv(g);
        for (int i = 0; i < N; i++) g[i] *= tw[i];
        butterfly(g);
        std::copy(g.begin(), g.end(), f.begin() + N);
    };
    while (true) {
        for (int i = 0; i < N; i++) {
            const mint p0 = P[2 * i], p1 = P[2 * i + 1], q0 = Q[2 * i], q1 = Q[2 * i + 1];
            // U = P(x) Q(-x) の w, -w での値は p0 q1, p1 q0。V = Q(x) Q(-x) はどちらも q0 q1
            const mint u0 = p0 * q1, u1 = p1 * q0;
            P[i] = (k & 1) ? (u0 - u1) * half_iw[i] : (u0 + u1) * inv2;
            Q[i] = q0 * q1;
        }
        k >>= 1;
        if (k == 0) break;
        extend(P), extend(Q);
    }
    // 定数項は長さ N の変換の値の和の 1/N なので、比は和の比になる
    mint p = 0, q = 0;
    for (int i = 0; i < N; i++) p += P[i], q += Q[i];
    return p / q;
}

}  // namespace internal_linear_recurrence

/**
 * @brief [x^k] P(x) / Q(x) (Q[0] != 0) を Bostan-Mori 法で求める O(d log d log k) (d は P, Q の長さ)
 * P(x) / Q(x) = P(x) Q(-x) / (Q(x) Q(-x)) で、分母は x^2 の多項式になるので、k の偶奇で分子の係数を半分に絞って k を半分にする
 */
template <typename mint>
mint bostan_mori(std::vector<mint> P, std::vector<mint> Q, long long k) {
    assert(!Q.empty() && Q[0] != 0 && k >= 0);
    if constexpr (internal_convolution::ntt_candidate<mint>()) {
        const int n = static_cast<int>(std::max(P.size(), Q.size()));
        if (k > 0 && n > 60) {
            int h = 0;
            while ((1 << h) < n) h++;
            if (internal_convolution::ntt_friendly<mint>(2 << h)) return internal_linear_recurrence::bostan_mori_ntt(std::move(P), std::move(Q), k, h);
        }
    }
    while (k > 0 && !P.empty()) {
        std::vector<mint> Qm = Q;
        for (size_t i = 1; i < Qm.size(); i += 2) Qm[i] = -Qm[i];
        std::vector<mint> U = convolution(P, Qm), V = convolution(Q, Qm);
        P.clear(), Q.clear();
        for (size_t i = k & 1; i < U.size(); i += 2) P.push_back(U[i]);
        for (size_t i = 0; i < V.size(); i += 2) Q.push_back(V[i]);
        k >>= 1;
    }
    if (P.empty()) return mint(0);
    // k = 0 なら P / Q の定数項
    return P[0] / Q[0];
}

/**
 * @brief a_i = c_1 a_{i-1} + c_2 a_{i-2} + ... + c_d a_{i-d} (i >= d) を満たす数列の第 k 項 (0-indexed) O(d log d log k)
 * @param a 初めの d 項 a_0, ..., a_{d-1}
 * @param c 係数 c_1, ..., c_d
 */
template <typename mint>
mint linear_recurrence_kth(const std::vector<mint>& a, const std::vector<mint>& c, long long k) {
    assert(a.size() == c.size() && k >= 0);
    const size_t d = c.size();
    if (d == 0) return mint(0);
    // 母関数 A(x) = P(x) / Q(x)。Q(x) = 1 - c_1 x - ... - c_d x^d、P(x) = A(x) Q(x) mod x^d
    std::vector<mint> Q(d + 1);
    Q[0] = 1;
    for (size_t j = 0; j < d; j++) Q[j + 1] = -c[j];
    std::vector<mint> P = convolution(a, Q);
    P.resize(d);
    return bostan_mori(P, Q, k);
}

/**
 * @brief 数列 a を満たす最短の線形漸化式の係数 c_1, ..., c_d (a_i = c_1 a_{i-1} + ... + c_d a_{i-d}、d <= i < |a|) O(|a|^2)
 * mint は体であること。長さ 2d 以上の項があれば、d 項間の漸化式を正しく復元できる
 */
template <typename mint>
std::vector<mint> berlekamp_massey(const std::vector<mint>& a) {
    // C(x) = 1 - c_1 x - ... : 今の漸化式、B(x): 最後に長さが変わる直前の漸化式
    std::vector<mint> C = {1}, B = {1};
    mint b = 1;  // B を使ったときの食い違い
    size_t len = 0, shift = 1;
    for (size_t i = 0; i < a.size(); i++) {
        // 今の漸化式で a_i を予測したときの食い違い
        mint delta = 0;
        for (size_t j = 0; j <= len && j < C.size(); j++) delta += C[j] * a[i - j];
        if (delta == 0) {
            shift++;
            continue;
        }
        // C(x) -= (delta / b) x^shift B(x) で食い違いを消す
        std::vector<mint> T = C;
        mint f = delta / b;
        if (C.size() < B.size() + shift) C.resize(B.size() + shift);
        for (size_t j = 0; j < B.size(); j++) C[j + shift] -= f * B[j];
        if (2 * len <= i) {
            len = i + 1 - len;
            B = T, b = delta, shift = 1;
        } else {
            shift++;
        }
    }
    C.resize(len + 1);
    std::vector<mint> res(len);
    for (size_t j = 0; j < len; j++) res[j] = -C[j + 1];
    return res;
}
