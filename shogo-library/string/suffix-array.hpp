#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/string/suffix-array.hpp

/**
 * @file suffix-array.hpp
 * @brief Suffix Array (SA-IS) と LCP 配列
 */

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

namespace internal_suffix_array {
// s の値は 0 以上 upper 以下
inline std::vector<int> sa_is(const std::vector<int>& s, int upper) {
    int n = s.size();
    if (n == 0) return {};
    if (n == 1) return {0};
    if (n == 2) return s[0] < s[1] ? std::vector<int>{0, 1} : std::vector<int>{1, 0};
    std::vector<int> sa(n);
    // ls[i]: s[i:] < s[i + 1:] なら true (S 型)、そうでなければ false (L 型)
    std::vector<bool> ls(n);
    for (int i = n - 2; i >= 0; i--) ls[i] = s[i] == s[i + 1] ? ls[i + 1] : s[i] < s[i + 1];
    // sum_l[c]: 先頭が c の L 型の接尾辞が入る位置の始まり、sum_s[c]: 先頭が c の S 型の接尾辞が入る位置の始まり
    std::vector<int> sum_l(upper + 1), sum_s(upper + 1);
    for (int i = 0; i < n; i++) {
        if (!ls[i])
            sum_s[s[i]]++;
        else
            sum_l[s[i] + 1]++;
    }
    for (int i = 0; i <= upper; i++) {
        sum_s[i] += sum_l[i];
        if (i < upper) sum_l[i + 1] += sum_s[i];
    }
    // LMS の並び lms から、誘導ソートで全体を並べる
    auto induce = [&](const std::vector<int>& lms) {
        std::fill(sa.begin(), sa.end(), -1);
        std::vector<int> buf(upper + 1);
        std::copy(sum_s.begin(), sum_s.end(), buf.begin());
        for (int d : lms) {
            if (d == n) continue;
            sa[buf[s[d]]++] = d;
        }
        std::copy(sum_l.begin(), sum_l.end(), buf.begin());
        sa[buf[s[n - 1]]++] = n - 1;
        for (int i = 0; i < n; i++) {
            int v = sa[i];
            if (v >= 1 && !ls[v - 1]) sa[buf[s[v - 1]]++] = v - 1;
        }
        std::copy(sum_l.begin(), sum_l.end(), buf.begin());
        for (int i = n - 1; i >= 0; i--) {
            int v = sa[i];
            if (v >= 1 && ls[v - 1]) sa[--buf[s[v - 1] + 1]] = v - 1;
        }
    };
    // LMS (左が L 型の S 型) の位置
    std::vector<int> lms_map(n + 1, -1), lms;
    for (int i = 1; i < n; i++) {
        if (!ls[i - 1] && ls[i]) {
            lms_map[i] = lms.size();
            lms.push_back(i);
        }
    }
    int m = lms.size();
    induce(lms);
    if (m) {
        std::vector<int> sorted_lms;
        sorted_lms.reserve(m);
        for (int v : sa) {
            if (lms_map[v] != -1) sorted_lms.push_back(v);
        }
        // LMS 部分文字列に番号を付け、その列の Suffix Array を再帰で求める
        std::vector<int> rec_s(m);
        int rec_upper = 0;
        rec_s[lms_map[sorted_lms[0]]] = 0;
        for (int i = 1; i < m; i++) {
            int l = sorted_lms[i - 1], r = sorted_lms[i];
            int end_l = lms_map[l] + 1 < m ? lms[lms_map[l] + 1] : n;
            int end_r = lms_map[r] + 1 < m ? lms[lms_map[r] + 1] : n;
            bool same = true;
            if (end_l - l != end_r - r) {
                same = false;
            } else {
                while (l < end_l && s[l] == s[r]) l++, r++;
                if (l == n || s[l] != s[r]) same = false;
            }
            if (!same) rec_upper++;
            rec_s[lms_map[sorted_lms[i]]] = rec_upper;
        }
        std::vector<int> rec_sa = sa_is(rec_s, rec_upper);
        for (int i = 0; i < m; i++) sorted_lms[i] = lms[rec_sa[i]];
        induce(sorted_lms);
    }
    return sa;
}
// 内部の int の Suffix Array を、返す long long の列に直す
inline std::vector<long long> to_long_long(const std::vector<int>& sa) { return std::vector<long long>(sa.begin(), sa.end()); }
}  // namespace internal_suffix_array

/**
 * @brief Suffix Array を求める O(n + upper)
 * 接尾辞 s[i:] を辞書順に並べたときの開始位置 i のリスト
 * @tparam T 整数型 (int や long long)
 * @param s 値は 0 以上 upper 以下
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, std::nullptr_t> = nullptr>
std::vector<long long> suffix_array(const std::vector<T>& s, long long upper) {
    assert(0 <= upper && upper < INT_MAX);
    for ([[maybe_unused]] T x : s) assert(0 <= static_cast<long long>(x) && static_cast<long long>(x) <= upper);
    if constexpr (std::is_same_v<T, int>) {
        return internal_suffix_array::to_long_long(internal_suffix_array::sa_is(s, static_cast<int>(upper)));
    } else {
        std::vector<int> s2(s.begin(), s.end());
        return internal_suffix_array::to_long_long(internal_suffix_array::sa_is(s2, static_cast<int>(upper)));
    }
}

/**
 * @brief Suffix Array を求める O(n log n)
 * 接尾辞 s[i:] を辞書順に並べたときの開始位置 i のリスト。値を座標圧縮してから求める
 * @tparam T operator< で比較できる型
 */
template <typename T>
std::vector<long long> suffix_array(const std::vector<T>& s) {
    int n = s.size();
    std::vector<int> idx(n);
    for (int i = 0; i < n; i++) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return s[a] < s[b]; });
    std::vector<int> s2(n);
    int now = 0;
    for (int i = 0; i < n; i++) {
        if (i && s[idx[i - 1]] < s[idx[i]]) now++;
        s2[idx[i]] = now;
    }
    return internal_suffix_array::to_long_long(internal_suffix_array::sa_is(s2, now));
}

/**
 * @brief 文字列の Suffix Array を求める O(n)
 * 接尾辞 s[i:] を辞書順に並べたときの開始位置 i のリスト
 */
inline std::vector<long long> suffix_array(const std::string& s) {
    int n = s.size();
    std::vector<int> s2(n);
    for (int i = 0; i < n; i++) s2[i] = static_cast<unsigned char>(s[i]);
    return internal_suffix_array::to_long_long(internal_suffix_array::sa_is(s2, 255));
}

/**
 * @brief LCP 配列を求める (Kasai 法) O(n)
 * 長さ n - 1 の配列 lcp を返す。lcp[i] は s[sa[i]:] と s[sa[i + 1]:] の最長共通接頭辞の長さ
 * @tparam R std::string や std::vector など、size() と operator[] を持つ型
 * @tparam I sa の要素の型 (long long や int)
 * @param sa s の Suffix Array
 */
template <typename R, typename I>
std::vector<long long> lcp_array(const R& s, const std::vector<I>& sa) {
    long long n = s.size();
    assert(static_cast<long long>(sa.size()) == n);
    if (n == 0) return {};
    std::vector<long long> rank(n);
    for (long long i = 0; i < n; i++) rank[sa[i]] = i;
    std::vector<long long> lcp(n - 1);
    // s[i:] と辞書順で 1 つ前の接尾辞との LCP は、s[i - 1:] のときの値から 1 を引いたもの以上になる
    for (long long i = 0, h = 0; i < n; i++) {
        if (h > 0) h--;
        if (rank[i] == 0) continue;
        long long j = sa[rank[i] - 1];
        while (j + h < n && i + h < n && s[j + h] == s[i + h]) h++;
        lcp[rank[i] - 1] = h;
    }
    return lcp;
}
