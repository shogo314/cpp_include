#pragma once

/**
 * @file divisor-transform.hpp
 * @brief 約数・倍数のゼータ変換、メビウス変換と GCD・LCM 畳み込み
 */

#include <cassert>
#include <vector>

#include "prime.hpp"
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/math/divisor-transform.hpp

/**
 * @brief 約数のゼータ変換 a[n] <- sum_{d | n} a[d] O(N log log N)
 * 添字 1, ..., N (N = a.size() - 1) を使う。a[0] は変えない
 */
template <typename T>
void divisor_zeta(std::vector<T>& a) {
    int n = static_cast<int>(a.size()) - 1;
    for (long long p : enumerate_primes(n)) {
        for (int i = 1; i <= n / p; i++) a[i * p] += a[i];
    }
}

/**
 * @brief 約数のメビウス変換 (divisor_zeta の逆変換) O(N log log N)
 * 添字 1, ..., N (N = a.size() - 1) を使う。a[0] は変えない
 */
template <typename T>
void divisor_mobius(std::vector<T>& a) {
    int n = static_cast<int>(a.size()) - 1;
    for (long long p : enumerate_primes(n)) {
        for (int i = n / p; i >= 1; i--) a[i * p] -= a[i];
    }
}

/**
 * @brief 倍数のゼータ変換 a[n] <- sum_{n | m, m <= N} a[m] O(N log log N)
 * 添字 1, ..., N (N = a.size() - 1) を使う。a[0] は変えない
 */
template <typename T>
void multiple_zeta(std::vector<T>& a) {
    int n = static_cast<int>(a.size()) - 1;
    for (long long p : enumerate_primes(n)) {
        for (int i = n / p; i >= 1; i--) a[i] += a[i * p];
    }
}

/**
 * @brief 倍数のメビウス変換 (multiple_zeta の逆変換) O(N log log N)
 * 添字 1, ..., N (N = a.size() - 1) を使う。a[0] は変えない
 */
template <typename T>
void multiple_mobius(std::vector<T>& a) {
    int n = static_cast<int>(a.size()) - 1;
    for (long long p : enumerate_primes(n)) {
        for (int i = 1; i <= n / p; i++) a[i] -= a[i * p];
    }
}

/**
 * @brief GCD 畳み込み c[k] = sum_{gcd(i, j) = k} a[i] b[j] (1 <= i, j <= N) O(N log log N)
 * a と b は同じ長さ N + 1 で、添字 0 は使わない。c[0] は T()
 */
template <typename T>
std::vector<T> gcd_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    multiple_zeta(a);
    multiple_zeta(b);
    for (int i = 1; i < static_cast<int>(a.size()); i++) a[i] *= b[i];
    multiple_mobius(a);
    if (!a.empty()) a[0] = T();
    return a;
}

/**
 * @brief LCM 畳み込み c[k] = sum_{lcm(i, j) = k} a[i] b[j] (1 <= i, j <= N、k <= N) O(N log log N)
 * a と b は同じ長さ N + 1 で、添字 0 は使わない。c[0] は T()。lcm が N を超える組は数えない
 */
template <typename T>
std::vector<T> lcm_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    divisor_zeta(a);
    divisor_zeta(b);
    for (int i = 1; i < static_cast<int>(a.size()); i++) a[i] *= b[i];
    divisor_mobius(a);
    if (!a.empty()) a[0] = T();
    return a;
}
