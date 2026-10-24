#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/lagrange-interpolation.hpp

/**
 * @file lagrange-interpolation.hpp
 * @brief ラグランジュ補間 (標本点が 0, 1, ..., n-1 のとき)
 */

#include <vector>

/**
 * @brief n - 1 次以下の多項式 f の f(0), f(1), ..., f(n-1) から f(x) を求める O(n)
 * mint は体で、n - 1 以下の正の整数が 0 にならないこと (法が n 以上の素数など)
 * f(x) = Σ_i y_i Π_{j != i} (x - j) / (i - j) を、(x - j) の前後からの積と階乗の逆数で計算する
 */
template <typename mint>
mint lagrange_interpolation(const std::vector<mint>& y, mint x) {
    const int n = y.size();
    if (n == 0) return mint(0);
    // pre[i] = Π_{j < i} (x - j)、suf[i] = Π_{j >= i} (x - j) (x が標本点でも、自分の因子を飛ばすので割り算はいらない)
    std::vector<mint> pre(n + 1, 1), suf(n + 1, 1);
    for (int i = 0; i < n; i++) pre[i + 1] = pre[i] * (x - i);
    for (int i = n - 1; i >= 0; i--) suf[i] = suf[i + 1] * (x - i);
    // inv_fact[i] = 1 / i!
    std::vector<mint> inv_fact(n, 1);
    mint fact = 1;
    for (int i = 1; i < n; i++) fact *= i;
    inv_fact[n - 1] = fact.inv();
    for (int i = n - 1; i > 0; i--) inv_fact[i - 1] = inv_fact[i] * i;
    mint res = 0;
    for (int i = 0; i < n; i++) {
        // Π_{j != i} (i - j) = i! (n - 1 - i)! (-1)^(n - 1 - i)
        mint t = y[i] * pre[i] * suf[i + 1] * inv_fact[i] * inv_fact[n - 1 - i];
        res += (n - 1 - i) % 2 == 0 ? t : -t;
    }
    return res;
}
