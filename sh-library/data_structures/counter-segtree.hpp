#pragma once
#include <cassert>
#include <map>
#include <vector>

class CounterSegTree {
    using S = int;
    int n, sz, height;
    std::vector<std::map<S, int>> data;

   public:
    explicit CounterSegTree(const std::vector<S>& v) {
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
    S get(int k) const {
        assert(0 <= k and k < n);
        return data[k + sz].begin()->first;
    }
    void set(int k, const S& x) {
        k += sz;
        S y = data[k].begin()->first;
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
    std::pair<S, int> lower_bound(int l, int r, int x) const {
        std::pair<S, int> ret = {{}, 0};
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
};
