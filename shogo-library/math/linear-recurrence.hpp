#pragma once

/**
 * @file linear-recurrence.hpp
 * @brief 線形漸化式 (Bostan-Mori による第 k 項、Berlekamp-Massey による漸化式の復元)
 */

#include <cassert>
#include <vector>

#include "convolution.hpp"
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/math/linear-recurrence.hpp

/**
 * @brief [x^k] P(x) / Q(x) (Q[0] != 0) を Bostan-Mori 法で求める O(d log d log k) (d は P, Q の長さ)
 * P(x) / Q(x) = P(x) Q(-x) / (Q(x) Q(-x)) で、分母は x^2 の多項式になるので、k の偶奇で分子の係数を半分に絞って k を半分にする
 */
template <typename mint>
mint bostan_mori(std::vector<mint> P, std::vector<mint> Q, long long k) {
    assert(!Q.empty() && Q[0] != 0 && k >= 0);
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
