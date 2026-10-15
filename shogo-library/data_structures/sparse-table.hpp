#pragma once

/**
 * @file sparse-table.hpp
 * @brief Sparse Table
 */

#include <cassert>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/data_structures/sparse-table.hpp

/**
 * @brief Sparse Table
 *
 * 静的な列の区間積を、前計算 O(n log n)、クエリ O(1) で求める
 * 例:
 * SparseTable<monoid::Min<int>> st(v);
 * st.prod(l, r);  // min(v[l], ..., v[r-1])
 *
 * @tparam M 冪等なモノイド (op(x, x) = x となるもの。monoid::Min、Max、Gcd、And、Or など)
 */
template <typename M>
class SparseTable {
   public:
    using S = typename M::S;

   private:
    M m;
    long long n;
    // table[k][i] は v[i] から v[i + 2^k - 1] までの積
    std::vector<std::vector<S>> table;
    std::vector<int> lg;  // lg[x] = floor(log2(x))
    S op(const S& a, const S& b) const { return m.op(a, b); }
    S e() const { return m.e(); }

   public:
    explicit SparseTable(const std::vector<S>& v = {}, M m = M()) : m(std::move(m)), n(v.size()), table(1, v), lg(n + 1, 0) {
        for (long long x = 2; x <= n; x++) lg[x] = lg[x / 2] + 1;
        for (int k = 1; (1LL << k) <= n; k++) {
            const auto& prev = table[k - 1];
            std::vector<S> cur(n - (1LL << k) + 1);
            for (long long i = 0; i + (1LL << k) <= n; i++) cur[i] = op(prev[i], prev[i + (1LL << (k - 1))]);
            table.push_back(std::move(cur));
        }
    }

    long long size() const { return n; }

    /**
     * @brief v[k]
     */
    S get(long long k) const {
        assert(0 <= k && k < n);
        return table[0][k];
    }
    /**
     * @brief v[l] から v[r-1] までの積 O(1)。l = r なら単位元
     */
    S prod(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return e();
        int k = lg[r - l];
        return op(table[k][l], table[k][r - (1LL << k)]);
    }
    S all_prod() const { return prod(0, n); }
};

template <typename M>
SparseTable(std::vector<typename M::S>, M) -> SparseTable<M>;
