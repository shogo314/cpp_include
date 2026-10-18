#pragma once

/**
 * @file longest-common-substring.hpp
 * @brief 最長共通部分文字列
 */

#include <algorithm>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

#include "suffix-array.hpp"
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/string/longest-common-substring.hpp

/**
 * @brief s と t の最長共通部分文字列 (連続する部分列) を 1 つ求める O((n + m) log(n + m))
 * s[i, i + len) == t[j, j + len) となる (i, j, len) を返す。共通部分がなければ (0, 0, 0)
 * 要素は operator< で比べる
 */
template <typename R1, typename R2>
std::tuple<long long, long long, long long> longest_common_substring(const R1& s, const R2& t) {
    using T = std::decay_t<decltype(*std::begin(s))>;
    std::vector<T> vs(std::begin(s), std::end(s)), vt(std::begin(t), std::end(t));
    long long n = vs.size(), m = vt.size();
    // 座標圧縮して、s、区切り、t の順に並べる。区切りは 1 回しか現れないので、LCP は区切りをまたがない
    std::vector<T> vals(vs);
    vals.insert(vals.end(), vt.begin(), vt.end());
    std::sort(vals.begin(), vals.end());
    vals.erase(std::unique(vals.begin(), vals.end(), [](const T& a, const T& b) { return !(a < b) && !(b < a); }),
               vals.end());
    long long sep = vals.size();
    std::vector<int> u;
    u.reserve(n + m + 1);
    for (const T& x : vs) u.push_back(std::lower_bound(vals.begin(), vals.end(), x) - vals.begin());
    u.push_back(sep);
    for (const T& x : vt) u.push_back(std::lower_bound(vals.begin(), vals.end(), x) - vals.begin());
    std::vector<long long> sa = suffix_array(u, sep);
    std::vector<long long> lcp = lcp_array(u, sa);
    // 辞書順で隣り合う接尾辞のうち、片方が s、もう片方が t から始まるものの LCP の最大
    long long i = 0, j = 0, len = 0;
    for (long long k = 0; k + 1 < (long long)sa.size(); k++) {
        long long a = sa[k], b = sa[k + 1];
        if (a > b) std::swap(a, b);
        if (a < n && n < b && lcp[k] > len) {
            i = a, j = b - n - 1, len = lcp[k];
        }
    }
    return {i, j, len};
}
