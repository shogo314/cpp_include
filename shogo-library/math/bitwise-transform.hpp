#pragma once
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/math/bitwise-transform.hpp

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

/**
 * @brief 部分集合畳み込み c[k] = sum_{i | j = k, i & j = 0} a[i] b[j] O(N log^2 N)
 * a と b は同じ長さ N で、N は 2 の累乗。メモリは O(N log N)
 */
template <typename T>
std::vector<T> subset_convolution(const std::vector<T>& a, const std::vector<T>& b) {
    int len = a.size();
    assert(a.size() == b.size() && internal_bitwise_transform::is_power_of_two(len));
    int n = 0;
    while ((1 << n) < len) n++;
    int w = n + 1;
    // f[s * w + k] は、要素数 k の集合だけを残したものの値 (ランク付きゼータ変換)
    std::vector<T> fa(static_cast<long long>(len) * w), fb(static_cast<long long>(len) * w);
    for (int s = 0; s < len; s++) {
        int k = __builtin_popcount(s);
        fa[static_cast<long long>(s) * w + k] = a[s];
        fb[static_cast<long long>(s) * w + k] = b[s];
    }
    for (int bit = 1; bit < len; bit <<= 1) {
        for (int s = 0; s < len; s++) {
            if (!(s & bit)) continue;
            T* x = &fa[static_cast<long long>(s) * w];
            T* y = &fb[static_cast<long long>(s) * w];
            const T* px = &fa[static_cast<long long>(s ^ bit) * w];
            const T* py = &fb[static_cast<long long>(s ^ bit) * w];
            for (int k = 0; k < w; k++) {
                x[k] += px[k];
                y[k] += py[k];
            }
        }
    }
    // 集合ごとに、要素数についての多項式として掛ける
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
    for (int bit = 1; bit < len; bit <<= 1) {
        for (int s = 0; s < len; s++) {
            if (!(s & bit)) continue;
            T* x = &fa[static_cast<long long>(s) * w];
            const T* px = &fa[static_cast<long long>(s ^ bit) * w];
            for (int k = 0; k < w; k++) x[k] -= px[k];
        }
    }
    std::vector<T> c(len);
    for (int s = 0; s < len; s++) c[s] = fa[static_cast<long long>(s) * w + __builtin_popcount(s)];
    return c;
}
