#pragma once
#include <vector>
#include <array>
#include <type_traits>
#include <algorithm>
#include <cmath>

template <typename U, class ADD_LEFT, class DELETE_LEFT, class ADD_RIGHT = ADD_LEFT, class DELETE_RIGHT = DELETE_LEFT>
struct Mo {
   private:
    int _n;
    std::vector<std::array<int, 3>> lri;
    U _e;
    ADD_LEFT _add_left;
    DELETE_LEFT _delete_left;
    ADD_RIGHT _add_right;
    DELETE_RIGHT _delete_right;

    static_assert(std::is_invocable_r_v<void, ADD_LEFT, U&, int>, "ADD_LEFT must be callable with (U&, int)");
    static_assert(std::is_invocable_r_v<void, DELETE_LEFT, U&, int>, "DELETE_LEFT must be callable with (U&, int)");
    static_assert(std::is_invocable_r_v<void, ADD_RIGHT, U&, int>, "ADD_RIGHT must be callable with (U&, int)");
    static_assert(std::is_invocable_r_v<void, DELETE_RIGHT, U&, int>, "DELETE_RIGHT must be callable with (U&, int)");

   public:
    Mo(int n,
       const U& e,
       const ADD_LEFT& add_left,
       const DELETE_LEFT& delete_left,
       const ADD_RIGHT& add_right,
       const DELETE_RIGHT delete_right) : _n(n), lri(), _e(e), _add_left(add_left), _delete_left(delete_left), _add_right(add_right), _delete_right(delete_right) {}
    Mo(int n,
       const U& e,
       const ADD_LEFT& add_left,
       const DELETE_LEFT& delete_left) : _n(n), lri(), _e(e), _add_left(add_left), _delete_left(delete_left), _add_right(add_left), _delete_right(delete_left) {}
    void query(int l, int r) {
        lri.push_back({l, r, (int)lri.size()});
    }
    template <typename T, class F>
    std::vector<T> solve(const F& f) {
        static_assert(std::is_invocable_r_v<T, F, U&>, "The function passed to solve must be callable with (U&) and return T");
        
        const int w = std::max<int>(1, 1.0 * _n / std::max<double>(1.0, std::sqrt(lri.size() * 2.0 / 3.0)));
        std::sort(lri.begin(), lri.end(), [=](const std::array<int, 3>& a, const std::array<int, 3>& b) {
            int aw = a[0] / w;
            int bw = b[0] / w;
            if (aw != bw) return aw < bw;
            if (aw & 1) return a[1] > b[1];
            return a[1] < b[1];
        });
        std::vector<T> res(lri.size());
        ll tmpl = 0, tmpr = 0;
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
