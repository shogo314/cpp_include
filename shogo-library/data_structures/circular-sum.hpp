#pragma once
// https://github.com/shogo314/shogo-library/blob/37ef4b20eeb8b4ec38d854e5a6d0bfae582ba3c6/src/data_structures/circular-sum.hpp

/**
 * @file circular-sum.hpp
 * @brief 円環上の区間和
 */

#include <cassert>
#include <vector>

/**
 * @brief 円環上の区間和
 *
 * 円環に並んだ a[0], ..., a[n-1] の区間和を O(1) で求める (static)
 * 例:
 * CircularSum cs(a);
 * cs.sum(l, r);       // l <= r なら a[l] + ... + a[r-1]
 * cs.sum(n - 1, 3);   // r < l なら 1 周またぐ: a[n-1] + a[0] + a[1] + a[2]
 *
 * @tparam T 値の型。T{} が 0 で、+ と - が使えればよい
 */
template <typename T>
class CircularSum {
   private:
    long long n;
    // s[i] = a[0] + ... + a[i-1]
    std::vector<T> s;

   public:
    explicit CircularSum(const std::vector<T>& v) : n(v.size()), s(n + 1, T{}) {
        for (long long i = 0; i < n; i++) s[i + 1] = s[i] + v[i];
    }

    long long size() const { return n; }

    /**
     * @brief l から時計回りに r の手前までの和 O(1)
     * l <= r なら [l, r)、r < l なら [l, n) と [0, r)。l == r は空 (0)
     * @pre 0 <= l, r <= n
     */
    T sum(long long l, long long r) const {
        assert(0 <= l && l <= n && 0 <= r && r <= n);
        if (l <= r) return s[r] - s[l];
        return s[n] - s[l] + s[r];
    }
    /**
     * @brief 1 周分の和 O(1)
     */
    T all_sum() const { return s[n]; }
};
