#pragma once
#include <algorithm>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

class MergeSortTree {
    using S = long long;
    using K = int;
    static S inv(const S& a) {
        return -a;
    }
    static S op(const S& a, const S& b) {
        return a + b;
    }
    static S e() {
        return 0;
    }
    int n, sz, height;
    std::vector<std::pair<K, S>> key_value_data;
    std::vector<S> cumulative_value;

    void initialize(const std::vector<K>& key, const std::vector<S>& value) {
        n = key.size();
        sz = 1;
        height = 1;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        key_value_data.assign(sz * height, {{}, e()});
        cumulative_value.assign(sz * height, {});
        for (int i = 0; i < n; i++) {
            key_value_data[i + sz * (height - 1)] = {key[i], value[i]};
            cumulative_value[i + sz * (height - 1)] = value[i];
        }
        int t = 1;
        // std::cout << "key_value_data[" << (height - 1) * sz << ":" << height * sz << "] =";
        // for (int i = 0; i < sz; i++) {
        //     std::cout << " {" << key_value_data[(height - 1) * sz + i].first << ", " << key_value_data[(height - 1) * sz + i].second << "}";
        // }
        // std::cout << std::endl;
        // std::cout << "cumulative_value[" << (height - 1) * sz << ":" << height * sz << "] =";
        // for (int i = 0; i < sz; i++) {
        //     std::cout << " " << cumulative_value[(height - 1) * sz + i];
        // }
        // std::cout << std::endl;
        for (int h = height - 1; h > 0; h--) {
            for (int i = 0; i < n; i += t * 2) {
                std::merge(key_value_data.begin() + h * sz + i,
                           key_value_data.begin() + h * sz + std::min(n, i + t),
                           key_value_data.begin() + h * sz + std::min(n, i + t),
                           key_value_data.begin() + h * sz + std::min(n, i + t * 2),
                           key_value_data.begin() + (h - 1) * sz + i,
                           [](const std::pair<K, S>& p1, const std::pair<K, S>& p2) { return p1.first < p2.first; });
                if (i < n) {
                    cumulative_value[(h - 1) * sz + i] = key_value_data[(h - 1) * sz + i].second;
                }
                for (int j = i + 1; j < std::min(n, i + t * 2); j++) {
                    cumulative_value[(h - 1) * sz + j] = op(cumulative_value[(h - 1) * sz + j - 1], key_value_data[(h - 1) * sz + j].second);
                }
            }
            // std::cout << "key_value_data[" << (h - 1) * sz << ":" << h * sz << "] =";
            // for (int i = 0; i < sz; i++) {
            //     std::cout << " {" << key_value_data[(h - 1) * sz + i].first << ", " << key_value_data[(h - 1) * sz + i].second << "}";
            // }
            // std::cout << std::endl;
            // std::cout << "cumulative_value[" << (h - 1) * sz << ":" << h * sz << "] =";
            // for (int i = 0; i < sz; i++) {
            //     std::cout << " " << cumulative_value[(h - 1) * sz + i];
            // }
            // std::cout << std::endl;
            t <<= 1;
        }
    }

    S _prod_section(int l, int r, std::optional<K> a, std::optional<K> b) const {
        // std::cerr << "l = " << l%sz << ", r = " << r%sz << std::endl;
        S ret = e();
        if (a.has_value()) {
            auto itr = std::lower_bound(cumulative_value.begin() + l, cumulative_value.begin() + r, a.value());
            if (itr == cumulative_value.begin() + l) {
                ret = op(ret, inv(*(--itr)));
            } else {
                ret = e();
            }
        } else {
            ret = e();
        }
        if (b.has_value()) {
            auto itr = std::upper_bound(cumulative_value.begin() + l, cumulative_value.begin() + r, b.value());
            ret = op(ret, *(--itr));
        } else {
            ret = op(ret, *(cumulative_value.begin() + r - 1));
        }
        return ret;
    }
    S _prod(int l, int r, std::optional<K> a, std::optional<K> b) const {
        S ret = e();
        int h = height - 1;
        int t = 1;
        while (l < r) {
            if (l & t) {
                ret += _prod_section(h * sz + l, h * sz + l + t, a, b);
                l += t;
            }
            if (r & t) {
                r -= t;
                ret += _prod_section(h * sz + r, h * sz + r + t, a, b);
            }
            h--;
            t <<= 1;
        }
        return ret;
    }

   public:
    MergeSortTree() = default;
    explicit MergeSortTree(const std::vector<std::pair<K, S>>& key_value) {
        std::vector<K> key;
        std::vector<S> value;
        key.reserve(key_value.size());
        value.reserve(key_value.size());
        for (size_t i = 0; i < key_value.size(); i++) {
            key.push_back(key_value[i].first);
            value.push_back(key_value[i].second);
        }
        this->initialize(key, value);
    }
    /**
     * key ソートする基準
     * value prodで計算する対象
     */
    MergeSortTree(const std::vector<K>& key, const std::vector<S>& value) {
        assert(key.size() == value.size());
        this->initialize(key, value);
    }

    /**
     * ploduct value[i] s.t. a <= key[i] < b , i in [l, r)
     */
    S prod(std::optional<int> l = std::nullopt, std::optional<int> r = std::nullopt, std::optional<K> a = std::nullopt, std::optional<K> b = std::nullopt) const {
        if (a.has_value() and b.has_value() and not(a.value() < b.value())) return e();
        if (l >= r) return e();
        return _prod(l.value_or(0), r.value_or(n), a, b);
    }

    std::vector<std::pair<K, S>> to_vector() const {
        std::vector<std::pair<K, S>> ret;
        ret.reserve(n);
        for (int i = 0; i < n; i++) {
            ret.push_back(key_value_data[(height - 1) * sz + i]);
        }
        return ret;
    }
};
