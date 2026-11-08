#pragma once

/**
 * @file disjoint-sparse-table.hpp
 * @brief 冪等性を仮定しない静的な区間積
 */

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
#include "../utility/bit.hpp"
#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/77ebb090933789a5c7f4dc5561b48ac9861e1e0b/src/data_structures/disjoint-sparse-table.hpp

template <typename M>
class DisjointSparseTable {
   public:
    using S = typename M::S;

   private:
    M m;
    long long n;
    std::vector<S> values;
    std::vector<std::vector<S>> table;

   public:
#if __cplusplus >= 202002L
    template <class R = std::initializer_list<S>>
        requires internal_traits::sized_constructible_range<R, S>
#else
    template <class R = std::initializer_list<S>, ENABLE_IF_T(internal_traits::is_sized_constructible_range<R, S>::value)>
#endif
    explicit DisjointSparseTable(R&& v = {}, M m = M()) : m(std::move(m)), n(internal_traits::range_size(v)) {
        values.reserve(n);
        for (auto&& x : v) values.emplace_back(std::forward<decltype(x)>(x));
        const int height = n <= 1 ? 0 : static_cast<int>(bit_width(n - 1));
        table.reserve(height);
        for (int k = 0; k < height; ++k) {
            const long long half = 1LL << k;
            std::vector<S> row(n, this->m.e());
            for (long long l = 0; l < n; l += 2 * half) {
                const long long mid = std::min(n, l + half), r = std::min(n, l + 2 * half);
                row[mid - 1] = values[mid - 1];
                for (long long i = mid - 1; i > l; --i) row[i - 1] = this->m.op(values[i - 1], row[i]);
                if (mid < r) {
                    row[mid] = values[mid];
                    for (long long i = mid + 1; i < r; ++i) row[i] = this->m.op(row[i - 1], values[i]);
                }
            }
            table.push_back(std::move(row));
        }
    }

    long long size() const { return n; }
    S get(long long k) const {
        assert(0 <= k && k < n);
        return values[k];
    }
    S prod(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return m.e();
        if (r - l == 1) return values[l];
        const int k = static_cast<int>(bit_width(l ^ (r - 1))) - 1;
        return m.op(table[k][l], table[k][r - 1]);
    }
    S all_prod() const { return prod(0, n); }
};

template <class R, typename M>
DisjointSparseTable(R&&, M) -> DisjointSparseTable<M>;
template <typename M>
DisjointSparseTable(std::initializer_list<typename M::S>, M) -> DisjointSparseTable<M>;
