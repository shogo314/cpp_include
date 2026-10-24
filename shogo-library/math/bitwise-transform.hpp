#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/bitwise-transform.hpp

/**
 * @file bitwise-transform.hpp
 * @brief 部分集合・上位集合のゼータ変換、メビウス変換とビット演算 (AND・OR・XOR) の畳み込み、部分集合畳み込み
 */

#include <cassert>
#include <type_traits>
#include <vector>

namespace internal_bitwise_transform {
// 長さが 2 の累乗か
inline bool is_power_of_two(long long n) { return n > 0 && (n & (n - 1)) == 0; }
}  // namespace internal_bitwise_transform

/**
 * @brief 部分集合のゼータ変換 a[S] <- sum_{T ⊆ S} a[T] O(N log N)
 * N = a.size() は 2 の累乗
 */
template <typename T>
void subset_zeta(std::vector<T>& a) {
    int n = a.size();
    assert(internal_bitwise_transform::is_power_of_two(n));
    for (int w = 1; w < n; w <<= 1) {
        for (int s = 0; s < n; s++) {
            if (s & w) a[s] += a[s ^ w];
        }
    }
}

/**
 * @brief 部分集合のメビウス変換 (subset_zeta の逆変換) O(N log N)
 * N = a.size() は 2 の累乗
 */
template <typename T>
void subset_mobius(std::vector<T>& a) {
    int n = a.size();
    assert(internal_bitwise_transform::is_power_of_two(n));
    for (int w = 1; w < n; w <<= 1) {
        for (int s = 0; s < n; s++) {
            if (s & w) a[s] -= a[s ^ w];
        }
    }
}

/**
 * @brief 上位集合のゼータ変換 a[S] <- sum_{S ⊆ T} a[T] O(N log N)
 * N = a.size() は 2 の累乗
 */
template <typename T>
void superset_zeta(std::vector<T>& a) {
    int n = a.size();
    assert(internal_bitwise_transform::is_power_of_two(n));
    for (int w = 1; w < n; w <<= 1) {
        for (int s = 0; s < n; s++) {
            if (s & w) a[s ^ w] += a[s];
        }
    }
}

/**
 * @brief 上位集合のメビウス変換 (superset_zeta の逆変換) O(N log N)
 * N = a.size() は 2 の累乗
 */
template <typename T>
void superset_mobius(std::vector<T>& a) {
    int n = a.size();
    assert(internal_bitwise_transform::is_power_of_two(n));
    for (int w = 1; w < n; w <<= 1) {
        for (int s = 0; s < n; s++) {
            if (s & w) a[s ^ w] -= a[s];
        }
    }
}

/**
 * @brief アダマール変換 a[S] <- sum_T (-1)^popcount(S & T) a[T] O(N log N)
 * N = a.size() は 2 の累乗。inverse = true なら逆変換 (最後に N で割る)
 */
template <typename T>
void walsh_hadamard(std::vector<T>& a, bool inverse = false) {
    int n = a.size();
    assert(internal_bitwise_transform::is_power_of_two(n));
    for (int w = 1; w < n; w <<= 1) {
        for (int s = 0; s < n; s++) {
            if (s & w) {
                T x = a[s ^ w], y = a[s];
                a[s ^ w] = x + y;
                a[s] = x - y;
            }
        }
    }
    if (inverse) {
        if constexpr (std::is_integral_v<T>) {
            // 結果は整数なので割り切れる
            for (T& x : a) x /= T(n);
        } else {
            T inv_n = T(1) / T(n);
            for (T& x : a) x *= inv_n;
        }
    }
}

/**
 * @brief AND 畳み込み c[k] = sum_{i & j = k} a[i] b[j] O(N log N)
 * a と b は同じ長さ N で、N は 2 の累乗
 */
template <typename T>
std::vector<T> and_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    superset_zeta(a);
    superset_zeta(b);
    for (int i = 0; i < static_cast<int>(a.size()); i++) a[i] *= b[i];
    superset_mobius(a);
    return a;
}

/**
 * @brief OR 畳み込み c[k] = sum_{i | j = k} a[i] b[j] O(N log N)
 * a と b は同じ長さ N で、N は 2 の累乗
 */
template <typename T>
std::vector<T> or_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    subset_zeta(a);
    subset_zeta(b);
    for (int i = 0; i < static_cast<int>(a.size()); i++) a[i] *= b[i];
    subset_mobius(a);
    return a;
}

/**
 * @brief XOR 畳み込み c[k] = sum_{i ^ j = k} a[i] b[j] O(N log N)
 * a と b は同じ長さ N で、N は 2 の累乗。T は N で割れること (modint、整数型、浮動小数点数)
 */
template <typename T>
std::vector<T> xor_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    walsh_hadamard(a);
    walsh_hadamard(b);
    for (int i = 0; i < static_cast<int>(a.size()); i++) a[i] *= b[i];
    walsh_hadamard(a, true);
    return a;
}

namespace internal_bitwise_transform {
// ランク付きゼータ変換。f[s * w + k] (w = n + 1) は、要素数 k の部分集合 T ⊆ s についての a[T] の和
template <typename T>
std::vector<T> ranked_zeta(const std::vector<T>& a, int n) {
    const int len = 1 << n, w = n + 1;
    std::vector<T> f(static_cast<long long>(len) * w);
    for (int s = 0; s < len; s++) f[static_cast<long long>(s) * w + __builtin_popcount(s)] = a[s];
    for (int bit = 1; bit < len; bit <<= 1) {
        for (int s = 0; s < len; s++) {
            if (!(s & bit)) continue;
            T* x = &f[static_cast<long long>(s) * w];
            const T* px = &f[static_cast<long long>(s ^ bit) * w];
            for (int k = 0; k < w; k++) x[k] += px[k];
        }
    }
    return f;
}
// ranked_zeta の逆変換をして、各 s について要素数 popcount(s) の値を取り出す
template <typename T>
std::vector<T> ranked_mobius(std::vector<T> f, int n) {
    const int len = 1 << n, w = n + 1;
    for (int bit = 1; bit < len; bit <<= 1) {
        for (int s = 0; s < len; s++) {
            if (!(s & bit)) continue;
            T* x = &f[static_cast<long long>(s) * w];
            const T* px = &f[static_cast<long long>(s ^ bit) * w];
            for (int k = 0; k < w; k++) x[k] -= px[k];
        }
    }
    std::vector<T> c(len);
    for (int s = 0; s < len; s++) c[s] = f[static_cast<long long>(s) * w + __builtin_popcount(s)];
    return c;
}
inline int log2_exact(long long len) {
    assert(is_power_of_two(len));
    int n = 0;
    while ((1LL << n) < len) n++;
    return n;
}
}  // namespace internal_bitwise_transform

/**
 * @brief 部分集合畳み込み c[k] = sum_{i | j = k, i & j = 0} a[i] b[j] O(N log^2 N)
 * ランク付きゼータ変換をして、集合ごとに要素数についての多項式として掛け、ランク付きメビウス変換で戻す
 * @note a と b の長さは同じ 2 のべき
 */
template <typename T>
std::vector<T> subset_convolution(const std::vector<T>& a, const std::vector<T>& b) {
    using namespace internal_bitwise_transform;
    assert(a.size() == b.size());
    const int n = log2_exact(a.size()), len = 1 << n, w = n + 1;
    auto fa = ranked_zeta(a, n);
    const auto fb = ranked_zeta(b, n);
    std::vector<T> tmp(w);
    for (int s = 0; s < len; s++) {
        T* x = &fa[static_cast<long long>(s) * w];
        const T* y = &fb[static_cast<long long>(s) * w];
        for (int k = 0; k < w; k++) {
            T sum = T();
            for (int i = 0; i <= k; i++) sum += x[i] * y[k - i];
            tmp[k] = sum;
        }
        for (int k = 0; k < w; k++) x[k] = tmp[k];
    }
    return ranked_mobius(std::move(fa), n);
}

/**
 * @brief 集合冪級数の exp: Σ_m a^m / m! (積は部分集合畳み込み) O(N log^2 N)
 * ランク付きゼータ変換をして、集合ごとに要素数についての多項式 p の exp を q' = p' q から求め、ランク付きメビウス変換で戻す
 * @note a[0] = 0。長さは 2 のべき。要素は体 (割り算を使う)
 */
template <typename T>
std::vector<T> set_power_series_exp(const std::vector<T>& a) {
    using namespace internal_bitwise_transform;
    const int n = log2_exact(a.size()), len = 1 << n, w = n + 1;
    assert(a[0] == T(0));
    auto f = ranked_zeta(a, n);
    std::vector<T> inv(w + 1), q(w), ip(w);
    for (int k = 1; k <= w; k++) inv[k] = T(1) / T(k);
    for (int s = 0; s < len; s++) {
        T* p = &f[static_cast<long long>(s) * w];
        // q[k] = (1 / k) Σ_{i=1}^{k} i p[i] q[k-i]
        for (int i = 1; i < w; i++) ip[i] = T(i) * p[i];
        q[0] = T(1);
        for (int k = 1; k < w; k++) {
            T sum = T();
            for (int i = 1; i <= k; i++) sum += ip[i] * q[k - i];
            q[k] = sum * inv[k];
        }
        for (int k = 0; k < w; k++) p[k] = q[k];
    }
    return ranked_mobius(std::move(f), n);
}

/**
 * @brief 集合冪級数の log (exp の逆) O(N log^2 N)
 * 集合ごとに要素数についての多項式 p の log を p q' = p' から求める
 * @note a[0] = 1。長さは 2 のべき。要素は体 (割り算を使う)
 */
template <typename T>
std::vector<T> set_power_series_log(const std::vector<T>& a) {
    using namespace internal_bitwise_transform;
    const int n = log2_exact(a.size()), len = 1 << n, w = n + 1;
    assert(a[0] == T(1));
    auto f = ranked_zeta(a, n);
    std::vector<T> inv(w + 1), q(w), iq(w);
    for (int k = 1; k <= w; k++) inv[k] = T(1) / T(k);
    for (int s = 0; s < len; s++) {
        T* p = &f[static_cast<long long>(s) * w];
        // p[0] = 1 なので、k q[k] = k p[k] - Σ_{i=1}^{k-1} i q[i] p[k-i]
        q[0] = T();
        for (int k = 1; k < w; k++) {
            T sum = T(k) * p[k];
            for (int i = 1; i < k; i++) sum -= iq[i] * p[k - i];
            q[k] = sum * inv[k];
            iq[k] = T(k) * q[k];
        }
        for (int k = 0; k < w; k++) p[k] = q[k];
    }
    return ranked_mobius(std::move(f), n);
}
