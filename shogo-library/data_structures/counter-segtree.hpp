#pragma once
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/data_structures/counter-segtree.hpp

/**
 * @file counter-segtree.hpp
 * @brief 区間内の値の個数を数えるセグメント木
 */

#include <cassert>
#include <map>
#include <optional>
#include <utility>
#include <vector>

template <typename T>
class CounterSegTree {
   public:
    using key_type = T;
    using mapped_type = long long;

   private:
    long long n, sz;
    int height;
    std::vector<std::map<key_type, int>> data;  // 各ノードの値ごとの個数 (メモリを抑えるため int で持つ)

   public:
    explicit CounterSegTree(const std::vector<key_type>& v) {
        n = v.size();
        sz = 1;
        height = 0;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        data.assign(sz * 2, {});
        for (long long i = 0; i < n; i++) {
            data[i + sz][v[i]] = 1;
        }
        for (long long i = sz - 1; i > 0; i--) {
            data[i] = data[2 * i];
            for (auto [s, c] : data[2 * i + 1]) {
                data[i][s] += c;
            }
        }
    }
    key_type get(long long k) const {
        assert(0 <= k && k < n);
        return data[k + sz].begin()->first;
    }
    void set(long long k, const key_type& x) {
        assert(0 <= k && k < n);
        k += sz;
        key_type y = data[k].begin()->first;
        while (k > 0) {
            auto itr = data[k].find(y);
            if (itr->second == 1) {
                data[k].erase(itr);
            } else {
                itr->second--;
            }
            data[k][x]++;
            k >>= 1;
        }
    }
    /**
     * @brief 区間 [l, r) で x 以上の最小の値とその個数。なければ std::nullopt
     */
    std::optional<std::pair<key_type, mapped_type>> lower_bound(long long l, long long r, key_type x) const {
        assert(0 <= l && l <= r && r <= n);
        std::pair<key_type, mapped_type> ret = {{}, 0};
        l += sz;
        r += sz;
        while (l < r) {
            if (l & 1) {
                auto itr = data[l].lower_bound(x);
                if (itr != data[l].end()) {
                    if (ret.second == 0) {
                        ret = *itr;
                    } else if (itr->first == ret.first) {
                        ret.second += itr->second;
                    } else if (itr->first < ret.first) {
                        ret = *itr;
                    }
                }
                ++l;
            }
            if (r & 1) {
                --r;
                auto itr = data[r].lower_bound(x);
                if (itr != data[r].end()) {
                    if (ret.second == 0) {
                        ret = *itr;
                    } else if (itr->first == ret.first) {
                        ret.second += itr->second;
                    } else if (itr->first < ret.first) {
                        ret = *itr;
                    }
                }
            }
            l >>= 1;
            r >>= 1;
        }
        if (ret.second == 0) return std::nullopt;
        return ret;
    }
    /**
     * @brief {x, 区間 [l, r) の x の個数}
     */
    std::pair<key_type, mapped_type> find(long long l, long long r, key_type x) const {
        assert(0 <= l && l <= r && r <= n);
        std::pair<key_type, mapped_type> ret = {x, 0};
        l += sz;
        r += sz;
        while (l < r) {
            if (l & 1) {
                auto itr = data[l].find(x);
                if (itr != data[l].end()) {
                    ret.second += itr->second;
                }
                ++l;
            }
            if (r & 1) {
                --r;
                auto itr = data[r].find(x);
                if (itr != data[r].end()) {
                    ret.second += itr->second;
                }
            }
            l >>= 1;
            r >>= 1;
        }
        return ret;
    }
};
