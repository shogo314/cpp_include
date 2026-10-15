#pragma once
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/others/mo.hpp

/**
 * @file mo.hpp
 * @brief Mo's algorithm
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <type_traits>
#include <vector>

template <typename U, class ADD_LEFT, class DELETE_LEFT, class ADD_RIGHT = ADD_LEFT, class DELETE_RIGHT = DELETE_LEFT>
class Mo {
   private:
    long long _n;
    std::vector<std::array<long long, 3>> lri;
    U _e;
    ADD_LEFT _add_left;
    DELETE_LEFT _delete_left;
    ADD_RIGHT _add_right;
    DELETE_RIGHT _delete_right;

    static_assert(std::is_invocable_r_v<void, ADD_LEFT, U&, long long>, "ADD_LEFT must be callable with (U&, long long)");
    static_assert(std::is_invocable_r_v<void, DELETE_LEFT, U&, long long>, "DELETE_LEFT must be callable with (U&, long long)");
    static_assert(std::is_invocable_r_v<void, ADD_RIGHT, U&, long long>, "ADD_RIGHT must be callable with (U&, long long)");
    static_assert(std::is_invocable_r_v<void, DELETE_RIGHT, U&, long long>, "DELETE_RIGHT must be callable with (U&, long long)");

   public:
    Mo(long long n,
       const U& e,
       const ADD_LEFT& add_left,
       const DELETE_LEFT& delete_left,
       const ADD_RIGHT& add_right,
       const DELETE_RIGHT& delete_right) : _n(n), lri(), _e(e), _add_left(add_left), _delete_left(delete_left), _add_right(add_right), _delete_right(delete_right) {}
    Mo(long long n,
       const U& e,
       const ADD_LEFT& add_left,
       const DELETE_LEFT& delete_left) : _n(n), lri(), _e(e), _add_left(add_left), _delete_left(delete_left), _add_right(add_left), _delete_right(delete_left) {}
    void query(long long l, long long r) {
        assert(0 <= l && l <= r && r <= _n);
        lri.push_back({l, r, (long long)lri.size()});
    }
    template <typename T, class F>
    std::vector<T> solve(const F& f) {
        static_assert(std::is_invocable_r_v<T, F, U&>, "The function passed to solve must be callable with (U&) and return T");

        const long long w = std::max<long long>(1, 1.0 * _n / std::max<double>(1.0, std::sqrt(lri.size() * 2.0 / 3.0)));
        std::sort(lri.begin(), lri.end(), [=](const std::array<long long, 3>& a, const std::array<long long, 3>& b) {
            long long aw = a[0] / w;
            long long bw = b[0] / w;
            if (aw != bw) return aw < bw;
            if (aw & 1) return a[1] > b[1];
            return a[1] < b[1];
        });
        std::vector<T> res(lri.size());
        long long tmpl = 0, tmpr = 0;
        U tmp = _e;
        for (auto [l, r, i] : lri) {
            while (tmpl > l) {
                tmpl--;
                _add_left(tmp, tmpl);
            }
            while (tmpr < r) {
                _add_right(tmp, tmpr);
                tmpr++;
            }
            while (tmpl < l) {
                _delete_left(tmp, tmpl);
                tmpl++;
            }
            while (tmpr > r) {
                tmpr--;
                _delete_right(tmp, tmpr);
            }
            res[i] = f(tmp);
        }
        return res;
    }
};
