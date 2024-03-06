#pragma once
#include <cassert>
#include <map>
#include <vector>

template <typename T>
class CounterSegTree {
    using key_type = T;
    using mapped_type = int;

    int n, sz, height;
    std::vector<std::map<key_type, mapped_type>> data;

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
        for (int i = 0; i < n; i++) {
            data[i + sz][v[i]] = 1;
        }
        for (int i = sz - 1; i > 0; i--) {
            data[i] = data[2 * i];
            for (auto [s, c] : data[2 * i + 1]) {
                data[i][s] += c;
            }
        }
    }
    key_type get(int k) const {
        assert(0 <= k and k < n);
        return data[k + sz].begin()->first;
    }
    void set(int k, const key_type& x) {
        k += sz;
        key_type y = data[k].begin()->first;
        while (k > 0) {
            auto itr = data[k].find(y);
            if (itr->second == 1) {
                data[k].erase(itr);
            } else {
                itr->second++;
            }
            data[k][x]++;
            k >>= 1;
        }
    }
    std::pair<key_type, mapped_type> lower_bound(int l, int r, key_type x) const {
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
        return ret;
    }
    std::pair<key_type, mapped_type> find(int l, int r, key_type x) const {
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
