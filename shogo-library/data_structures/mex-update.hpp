#pragma once
// https://github.com/shogo314/shogo-library/blob/main/src/data_structures/mex-update.hpp

/**
 * @file mex-update.hpp
 * @brief 1点更新・全体の mex 取得
 */

#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <set>
#include <vector>

template <typename T>
class MexUpdate {
   public:
    using value_type = T;

   private:
    inline constexpr static value_type e = std::numeric_limits<value_type>::max();

    int n;
    std::vector<value_type> v;
    std::set<int> s;
    std::map<int, std::size_t> m;

   public:
    MexUpdate() = default;
    explicit MexUpdate(int n) : n(n), v(n, e) {
        assert(n > 0);
        assert(n + 1 < e);
        std::set<int>::iterator hint = s.end();
        for (int i = n; i >= 0; --i) {
            hint = s.emplace_hint(hint, i);
        }
    }
    explicit MexUpdate(const std::vector<value_type>& init) : n(init.size()), v(init) {
        assert(n > 0);
        assert(n + 1 < e);
        std::set<int>::iterator hint = s.end();
        for (int i = n; i >= 0; --i) {
            hint = s.emplace_hint(hint, i);
        }
        for (const auto& i : init) {
            if (0 <= i && i <= n) {
                m[i]++;
                s.erase(i);
            }
        }
    }
    void set(int k, const value_type& x) {
        assert(0 <= k && k < n);
        if (0 <= v[k] && v[k] <= n) {
            int v_k = v[k];
            auto itr = m.find(v_k);
            if (itr->second > 1) {
                itr->second--;
            } else {
                m.erase(itr);
                s.insert(v_k);
            }
        }
        if (0 <= x && x <= n) {
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
