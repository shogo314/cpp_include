#pragma once

/**
 * @file wildcard-matching.hpp
 * @brief ワイルドカードつきのパターンマッチング
 */

#include <algorithm>
#include <iterator>
#include <map>
#include <random>
#include <vector>

#include "../math/convolution.hpp"
#include "../math/modint.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/string/wildcard-matching.hpp

/**
 * @brief text の中で pattern と一致する開始位置を昇順に返す。wildcard の要素はどの要素とも一致する (text と pattern の両方で使える) O((N + M) log (N + M))
 *
 * 要素を乱数の値 a に、wildcard を 0 に置き換えると、位置 i で一致するのは Σ_j a_{i+j} b_j (a_{i+j} - b_j)^2 = 0 のとき
 * 3 回の畳み込みでまとめて求める。乱数を使うので、一致しない位置を一致すると判定する確率が 4 / 998244353 ほどある
 *
 * @param text 文字列や列
 * @param pattern 文字列や列
 * @param wildcard ワイルドカードの要素 (文字列なら '*' など)
 */
template <typename R1, typename R2, typename T>
std::vector<long long> wildcard_pattern_matching(const R1& text, const R2& pattern, const T& wildcard) {
    using mint = modint998244353;
    const long long n = std::size(text), m = std::size(pattern);
    if (m > n) return {};
    if (m == 0) {
        std::vector<long long> res(n + 1);
        for (long long i = 0; i <= n; i++) res[i] = i;
        return res;
    }
    // 要素ごとに 0 でない乱数の値を割り当てる
    static std::mt19937_64 rng(std::random_device{}());
    std::map<T, mint> value;
    auto to_value = [&](const T& c) {
        if (c == wildcard) return mint(0);
        auto it = value.find(c);
        if (it != value.end()) return it->second;
        return value[c] = mint(rng() % (mint::mod() - 1) + 1);
    };
    std::vector<mint> a1(n), a2(n), a3(n), b1(m), b2(m), b3(m);
    {
        long long i = 0;
        for (const auto& c : text) {
            a1[i] = to_value(c), a2[i] = a1[i] * a1[i], a3[i] = a2[i] * a1[i];
            i++;
        }
    }
    {
        // pattern は逆順にして畳み込む
        long long j = m - 1;
        for (const auto& c : pattern) {
            b1[j] = to_value(c), b2[j] = b1[j] * b1[j], b3[j] = b2[j] * b1[j];
            j--;
        }
    }
    auto c31 = convolution(a3, b1), c22 = convolution(a2, b2), c13 = convolution(a1, b3);
    std::vector<long long> res;
    for (long long i = 0; i + m <= n; i++) {
        if (c31[i + m - 1] - 2 * c22[i + m - 1] + c13[i + m - 1] == 0) res.push_back(i);
    }
    return res;
}
