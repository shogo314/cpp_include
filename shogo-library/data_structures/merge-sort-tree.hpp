#pragma once

/**
 * @file merge-sort-tree.hpp
 * @brief マージソートツリー
 */
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "../algebra/group.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/data_structures/merge-sort-tree.hpp

/**
 * @brief 区間の閾値以内の値の積
 *
 * @tparam G 値の可換群 (algebra/group.hpp)
 * @tparam K key_type ソートに使う型
 * @tparam Comp Kを比較するファンクタ
 */
template <typename G = group::Sum<long long>,
          typename K = long long,
          class Comp = std::less<K>>
class MergeSortTree {
   public:
    using value_type = typename G::S;
    using key_type = K;

    inline constexpr static auto comp = Comp();

   private:
    G g;
    value_type op(const value_type& x, const value_type& y) const { return g.op(x, y); }
    value_type e() const { return g.e(); }
    value_type inv(const value_type& x) const { return g.inv(x); }

   private:
    long long n, sz;
    int height;
    std::vector<key_type> key_data;
    std::vector<value_type> cumulative_value;

    void initialize(const std::vector<value_type>& value, const std::vector<key_type>& key) {
        n = key.size();
        sz = 1;
        height = 1;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        key_data.assign(sz * height, {});
        std::vector<value_type> value_data(sz * height, e());
        cumulative_value.assign(sz * height, {});
        for (long long i = 0; i < n; i++) {
            key_data[(height - 1) * sz + i] = key[i];
            value_data[(height - 1) * sz + i] = value[i];
            cumulative_value[(height - 1) * sz + i] = value[i];
        }
        long long t = 1;
        for (int h = height - 1; h > 0; h--) {
            for (long long i = 0; i < n; i += t * 2) {
                long long j1 = h * sz + i;
                long long j2 = h * sz + std::min(n, i + t);
                long long j0 = (h - 1) * sz + i;
                long long last1 = j2;
                long long last2 = h * sz + std::min(n, i + t * 2);
                while (j1 != last1 || j2 != last2) {
                    if (j1 == last1 || (j2 < last2 && comp(key_data[j2], key_data[j1]))) {
                        key_data[j0] = key_data[j2];
                        value_data[j0] = value_data[j2];
                        j0++;
                        j2++;
                    } else {
                        key_data[j0] = key_data[j1];
                        value_data[j0] = value_data[j1];
                        j0++;
                        j1++;
                    }
                }
                cumulative_value[(h - 1) * sz + i] = value_data[(h - 1) * sz + i];
                for (long long j = i + 1; j < std::min(n, i + t * 2); j++) {
                    cumulative_value[(h - 1) * sz + j] = op(cumulative_value[(h - 1) * sz + j - 1], value_data[(h - 1) * sz + j]);
                }
            }
            t <<= 1;
        }
    }

    value_type prod_section(long long l, long long r, std::optional<key_type> a, std::optional<key_type> b) const {
        value_type ret = cumulative_value[r - 1];
        if (b.has_value()) {
            long long i = std::lower_bound(key_data.begin() + l, key_data.begin() + r, b.value(), comp) - key_data.begin();
            if (i != l) {
                ret = cumulative_value[i - 1];
            } else {
                ret = e();
            }
        }
        if (a.has_value()) {
            long long i = std::lower_bound(key_data.begin() + l, key_data.begin() + r, a.value(), comp) - key_data.begin();
            if (i != l) {
                ret = op(ret, inv(cumulative_value[i - 1]));
            }
        }
        return ret;
    }

   public:
    MergeSortTree() = default;
    /**
     * @param value_key valueとkeyのpairのvector
     */
    explicit MergeSortTree(const std::vector<std::pair<value_type, key_type>>& value_key, G g = G()) : g(std::move(g)) {
        std::vector<key_type> key;
        std::vector<value_type> value;
        key.reserve(value_key.size());
        value.reserve(value_key.size());
        for (std::size_t i = 0; i < value_key.size(); i++) {
            value.push_back(value_key[i].first);
            key.push_back(value_key[i].second);
        }
        this->initialize(value, key);
    }
    /**
     * @param value prodで計算する対象
     * @param key ソートする基準
     */
    MergeSortTree(const std::vector<value_type>& value, const std::vector<key_type>& key, G g = G()) : g(std::move(g)) {
        assert(key.size() == value.size());
        this->initialize(value, key);
    }

    /**
     * @brief product value[i] s.t. a <= key[i] < b , i in [l, r)
     *
     * @param l 半開区間の開始 nulloptの場合は0
     * @param r 半開区間の終端 0<=l<=r<=n nulloptの場合はn
     * @param a nulloptの場合は負の無限大
     * @param b nulloptの場合は正の無限大
     */
    value_type prod(std::optional<long long> l_ = std::nullopt, std::optional<long long> r_ = std::nullopt, std::optional<key_type> a = std::nullopt, std::optional<key_type> b = std::nullopt) const {
        long long l = l_.value_or(0), r = r_.value_or(n);
        assert(0 <= l && l <= r && r <= n);
        if (a.has_value() && b.has_value() && !comp(a.value(), b.value())) return e();
        value_type ret = e();
        int h = height - 1;
        long long t = 1;
        while (l < r) {
            if (l & t) {
                ret = op(ret, prod_section(h * sz + l, h * sz + l + t, a, b));
                l += t;
            }
            if (r & t) {
                r -= t;
                ret = op(ret, prod_section(h * sz + r, h * sz + r + t, a, b));
            }
            h--;
            t <<= 1;
        }
        return ret;
    }
};

template <typename G, typename K>
MergeSortTree(std::vector<std::pair<typename G::S, K>>, G) -> MergeSortTree<G, K>;
template <typename G, typename K>
MergeSortTree(std::vector<typename G::S>, std::vector<K>, G) -> MergeSortTree<G, K>;
