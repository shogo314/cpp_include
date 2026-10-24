#pragma once

/**
 * @file longest-common-substring.hpp
 * @brief 最長共通部分文字列と、最長の繰り返し部分文字列
 */

#include <algorithm>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "suffix-array.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/string/longest-common-substring.hpp

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
    for (long long k = 0; k + 1 < static_cast<long long>(sa.size()); k++) {
        long long a = sa[k], b = sa[k + 1];
        if (a > b) std::swap(a, b);
        if (a < n && n < b && lcp[k] > len) {
            i = a, j = b - n - 1, len = lcp[k];
        }
    }
    return {i, j, len};
}

/**
 * @brief s の中に 2 回以上現れる (重なってよい) 最長の部分文字列を 1 つ求める O(n log n)
 * s[i, i + len) == s[j, j + len) かつ i < j となる (i, j, len) を返す。2 回以上現れる部分がなければ (0, 0, 0)
 * 辞書順で隣り合う接尾辞の LCP の最大。要素は operator< で比べる
 */
template <typename R>
std::tuple<long long, long long, long long> longest_repeated_substring(const R& s) {
    using T = std::decay_t<decltype(*std::begin(s))>;
    std::vector<T> v(std::begin(s), std::end(s));
    std::vector<long long> sa = suffix_array(v);
    std::vector<long long> lcp = lcp_array(v, sa);
    long long i = 0, j = 0, len = 0;
    for (long long k = 0; k < static_cast<long long>(lcp.size()); k++) {
        if (lcp[k] > len) i = std::min(sa[k], sa[k + 1]), j = std::max(sa[k], sa[k + 1]), len = lcp[k];
    }
    return {i, j, len};
}

/**
 * @brief s の中に重ならずに 2 回以上現れる最長の部分文字列を 1 つ求める O(n log n)
 * s[i, i + len) == s[j, j + len) かつ i + len <= j となる (i, j, len) を返す。なければ (0, 0, 0)
 *
 * 長さ L のものがあれば L - 1 のものもあるので、L で二分探索する。L を固定したら、辞書順で隣り合う LCP が
 * L 以上の連続した区間ごとに、開始位置の最小と最大の差が L 以上かを見る。要素は operator< で比べる
 */
template <typename R>
std::tuple<long long, long long, long long> longest_non_overlapping_repeated_substring(const R& s) {
    using T = std::decay_t<decltype(*std::begin(s))>;
    std::vector<T> v(std::begin(s), std::end(s));
    const long long n = v.size();
    std::vector<long long> sa = suffix_array(v);
    std::vector<long long> lcp = lcp_array(v, sa);
    // 長さ L で重ならない 2 か所があれば、その開始位置 (i, j) を返す。なければ (-1, -1)
    auto find = [&](long long L) -> std::pair<long long, long long> {
        long long lo = sa.empty() ? 0 : sa[0], hi = lo;
        for (long long k = 0; k < n; k++) {
            if (k > 0 && lcp[k - 1] >= L) {
                lo = std::min(lo, sa[k]), hi = std::max(hi, sa[k]);
            } else {
                lo = hi = sa[k];
            }
            if (hi - lo >= L) return {lo, hi};
        }
        return {-1, -1};
    };
    // find(ok) が見つかり、find(ng) が見つからない
    long long ok = 0, ng = n / 2 + 1;
    while (ng - ok > 1) {
        long long mid = (ok + ng) / 2;
        (find(mid).first != -1 ? ok : ng) = mid;
    }
    if (ok == 0) return {0, 0, 0};
    auto [i, j] = find(ok);
    return {i, j, ok};
}
