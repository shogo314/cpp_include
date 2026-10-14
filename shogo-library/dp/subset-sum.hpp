#pragma once
// https://github.com/shogo314/shogo-library/blob/main/src/dp/subset-sum.hpp

/**
 * @file subset-sum.hpp
 * @brief 部分和問題 (作れる和の判定と復元)
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace internal_subset_sum {
// bits (S + 1 ビット) について bits |= bits << s を行う。新しく立ったビットごとに f(位置) を呼ぶ
template <typename F>
void shift_or(std::vector<unsigned long long>& bits, long long S, long long s, F f) {
    int words = bits.size();
    long long q = s / 64;
    int r = s % 64;
    for (long long k = words - 1; k >= q; k--) {
        unsigned long long val = bits[k - q] << r;
        if (r && k - q - 1 >= 0) val |= bits[k - q - 1] >> (64 - r);
        if (k == words - 1 && (S + 1) % 64) val &= (1ULL << ((S + 1) % 64)) - 1;  // S を超える位置は捨てる
        unsigned long long added = val & ~bits[k];
        bits[k] |= val;
        while (added) {
            f(k * 64 + __builtin_ctzll(added));
            added &= added - 1;
        }
    }
}

// bitset で作れる和を求める。from があれば、和 s を初めて作れるようになったときに使った要素の添字を記録する
inline std::vector<unsigned long long> build(const std::vector<long long>& a, long long S, std::vector<int>* from) {
    assert(S >= 0);
    std::vector<unsigned long long> bits(S / 64 + 1);
    bits[0] = 1;
    if (from) from->assign(S + 1, -1);
    for (int i = 0; i < (int)a.size(); i++) {
        assert(a[i] >= 0);
        if (a[i] == 0 || a[i] > S) continue;
        if (from)
            shift_or(bits, S, a[i], [&](long long s) { (*from)[s] = i; });
        else
            shift_or(bits, S, a[i], [](long long) {});
    }
    return bits;
}
inline bool test(const std::vector<unsigned long long>& bits, long long s) { return bits[s / 64] >> (s % 64) & 1; }
// 作れる和 s から from をたどって、添字の昇順のリストにする
// s - a[from[s]] は from[s] より前の要素だけで作れていたので、添字は真に減っていく
inline std::vector<int> trace(const std::vector<long long>& a, const std::vector<int>& from, long long s) {
    std::vector<int> res;
    for (; s > 0; s -= a[res.back()]) res.push_back(from[s]);
    return std::vector<int>(res.rbegin(), res.rend());
}
// 作れる和のうち S 以下の最大値
inline long long max_bit(const std::vector<unsigned long long>& bits, long long S) {
    for (long long k = S / 64; k >= 0; k--) {
        unsigned long long x = bits[k];
        if (k == S / 64 && (S + 1) % 64) x &= (1ULL << ((S + 1) % 64)) - 1;
        if (x) return k * 64 + 63 - __builtin_clzll(x);
    }
    return 0;
}

// 要素 [l, r) の部分集合のうち和が S 以下のものを、(和, 選んだ要素のビット列) の和の昇順で列挙する
inline std::vector<std::pair<long long, unsigned long long>> enumerate_half(const std::vector<long long>& a, int l, int r, long long S) {
    std::vector<std::pair<long long, unsigned long long>> res = {{0, 0}}, add, merged;
    for (int i = l; i < r; i++) {
        add.clear();
        for (auto [x, mask] : res) {
            if (a[i] <= S - x) add.emplace_back(x + a[i], mask | 1ULL << (i - l));
        }
        merged.resize(res.size() + add.size());
        std::merge(res.begin(), res.end(), add.begin(), add.end(), merged.begin());
        std::swap(res, merged);
    }
    return res;
}
// 半分全列挙で S 以下の最大の和を求め、(和, 前半のビット列, 後半のビット列) を返す
struct MitmResult {
    long long sum;
    unsigned long long left, right;
};
inline MitmResult meet_in_the_middle(const std::vector<long long>& a, long long S) {
    assert(S >= 0);
    int n = a.size();
    assert(n <= 128 && "subset_sum_max_meet_in_the_middle: too many elements");
    for (long long x : a) assert(x >= 0);
    auto p = enumerate_half(a, 0, n / 2, S), q = enumerate_half(a, n / 2, n, S);
    MitmResult res = {0, 0, 0};
    // p を和の昇順に見ると、組める q の和の上限は減っていく
    int j = q.size() - 1;
    for (auto [x, mask] : p) {
        while (q[j].first > S - x) j--;  // q[0] は和 0 なので j >= 0 のまま
        if (x + q[j].first > res.sum) res = {x + q[j].first, mask, q[j].second};
    }
    return res;
}
inline long double cost_bitset(int n, long long S) { return (long double)(n + 1) * ((long double)S / 64 + 1); }
inline long double cost_meet_in_the_middle(int n) {
    if (n > 120) return std::numeric_limits<long double>::infinity();
    return 4 * (long double)(1ULL << ((n + 1) / 2));
}
}  // namespace internal_subset_sum

/**
 * @brief a の部分集合 (各要素は 1 回まで) の和として、0, 1, ..., S のそれぞれを作れるか O(N S / 64)
 * @param a 0 以上の整数の列
 * @return 長さ S + 1 の列。res[s] は和 s を作れるか
 */
inline std::vector<bool> subset_sum(const std::vector<long long>& a, long long S) {
    auto bits = internal_subset_sum::build(a, S, nullptr);
    std::vector<bool> res(S + 1);
    for (long long s = 0; s <= S; s++) res[s] = internal_subset_sum::test(bits, s);
    return res;
}

/**
 * @brief a の部分集合 (各要素は 1 回まで) で和がちょうど S になるものを 1 つ求め、その添字を昇順に返す O(N S / 64 + S)
 * @param a 0 以上の整数の列
 * @return 作れなければ std::nullopt
 */
inline std::optional<std::vector<int>> subset_sum_restore(const std::vector<long long>& a, long long S) {
    std::vector<int> from;
    auto bits = internal_subset_sum::build(a, S, &from);
    if (!internal_subset_sum::test(bits, S)) return std::nullopt;
    return internal_subset_sum::trace(a, from, S);
}

/**
 * @brief a の部分集合 (各要素は 1 回まで) の和のうち、S 以下の最大値 (bitset) O(N S / 64)
 * @param a 0 以上の整数の列
 */
inline long long subset_sum_max_bitset(const std::vector<long long>& a, long long S) {
    return internal_subset_sum::max_bit(internal_subset_sum::build(a, S, nullptr), S);
}

/**
 * @brief a の部分集合 (各要素は 1 回まで) の和のうち、S 以下の最大値 (半分全列挙) O(2^(N/2))
 * N が 40 程度までなら、S や要素が大きくてもよい
 * @param a 0 以上の整数の列
 */
inline long long subset_sum_max_meet_in_the_middle(const std::vector<long long>& a, long long S) {
    return internal_subset_sum::meet_in_the_middle(a, S).sum;
}

/**
 * @brief a の部分集合 (各要素は 1 回まで) の和のうち、S 以下の最大値
 *
 * 計算量が小さい方を選ぶ: subset_sum_max_bitset O(N S / 64) か subset_sum_max_meet_in_the_middle O(2^(N/2))
 * @param a 0 以上の整数の列
 */
inline long long subset_sum_max(const std::vector<long long>& a, long long S) {
    using namespace internal_subset_sum;
    if (cost_bitset(a.size(), S) <= cost_meet_in_the_middle(a.size())) return subset_sum_max_bitset(a, S);
    return subset_sum_max_meet_in_the_middle(a, S);
}

/**
 * @brief 和が S 以下で最大になる部分集合 (各要素は 1 回まで) を 1 つ求め、その添字を昇順に返す
 *
 * subset_sum_max と同じく、bitset O(N S / 64 + S) と半分全列挙 O(2^(N/2)) のうち計算量が小さい方を選ぶ
 * @param a 0 以上の整数の列
 */
inline std::vector<int> subset_sum_max_restore(const std::vector<long long>& a, long long S) {
    using namespace internal_subset_sum;
    int n = a.size();
    if (cost_bitset(n, S) <= cost_meet_in_the_middle(n)) {
        std::vector<int> from;
        auto bits = build(a, S, &from);
        return trace(a, from, max_bit(bits, S));
    }
    MitmResult r = meet_in_the_middle(a, S);
    std::vector<int> res;
    for (int i = 0; i < n / 2; i++) {
        if (r.left >> i & 1) res.push_back(i);
    }
    for (int i = n / 2; i < n; i++) {
        if (r.right >> (i - n / 2) & 1) res.push_back(i);
    }
    return res;
}
