#pragma once
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/data_structures/mex-update.hpp

/**
 * @file mex-update.hpp
 * @brief 1点更新・全体の mex 取得
 */

#include <cassert>
#include <limits>
#include <map>
#include <set>
#include <type_traits>
#include <vector>

template <typename T>
class MexUpdate {
    static_assert(std::is_integral_v<T>, "MexUpdate: T must be an integer type");

   public:
    using value_type = T;

   private:
    inline constexpr static value_type e = std::numeric_limits<value_type>::max();

    long long n;
    std::vector<value_type> v;
    std::set<int> s;
    std::map<int, int> m;

    // x が 0 以上 n 以下か (T が符号なしでも警告が出ないように比べる)
    bool in_range(const value_type& x) const {
        if constexpr (std::is_signed_v<value_type>) {
            if (x < 0) return false;
        }
        return static_cast<unsigned long long>(x) <= static_cast<unsigned long long>(n);
    }

   public:
    MexUpdate() = default;
    explicit MexUpdate(long long n) : n(n), v(n, e) {
        assert(n > 0);
        assert(static_cast<unsigned long long>(n) + 1 < static_cast<unsigned long long>(e));
        std::set<int>::iterator hint = s.end();
        for (long long i = n; i >= 0; --i) {
            hint = s.emplace_hint(hint, static_cast<int>(i));
        }
    }
    explicit MexUpdate(const std::vector<value_type>& init) : n(init.size()), v(init) {
        assert(n > 0);
        assert(static_cast<unsigned long long>(n) + 1 < static_cast<unsigned long long>(e));
        std::set<int>::iterator hint = s.end();
        for (long long i = n; i >= 0; --i) {
            hint = s.emplace_hint(hint, static_cast<int>(i));
        }
        for (const auto& i : init) {
            if (in_range(i)) {
                m[i]++;
                s.erase(i);
            }
        }
    }
    void set(long long k, const value_type& x) {
        assert(0 <= k && k < n);
        if (in_range(v[k])) {
            int v_k = v[k];
            auto itr = m.find(v_k);
            if (itr->second > 1) {
                itr->second--;
            } else {
                m.erase(itr);
                s.insert(v_k);
            }
        }
        if (in_range(x)) {
            int xx = x;
            auto itr = m.find(xx);
            if (itr == m.end()) {
                m[xx]++;
                s.erase(xx);
            } else {
                itr->second++;
            }
        }
        v[k] = x;
    }
    value_type mex() const {
        return *s.begin();
    }
};
