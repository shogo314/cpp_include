#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/dp/slope-trick.hpp

/**
 * @file slope-trick.hpp
 * @brief Slope Trick
 */

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 凸な区分線形関数 f を、傾きが変わる点の多重集合として持つ (Slope Trick)
 *
 * 最小値をとる区間より左の点を L (最大ヒープ)、右の点を R (最小ヒープ) に入れ、平行移動はまとめて足す
 * はじめは f(x) = 0
 * 例:
 * SlopeTrick<long long> st;
 * for (long long a : A) {
 *     st.prefix_min();  // 非減少にするので、前の値以下ならどこでもよい
 *     st.add_abs(a);
 * }
 * long long cost = st.min();
 *
 * @tparam T 値の型 (符号付き整数)
 */
template <typename T>
class SlopeTrick {
    T min_f = 0, add_l = 0, add_r = 0;
    std::priority_queue<T> L;
    std::priority_queue<T, std::vector<T>, std::greater<T>> R;

    void push_l(T a) { L.push(a - add_l); }
    void push_r(T a) { R.push(a - add_r); }
    T top_l() const { return L.empty() ? -inf() : L.top() + add_l; }
    T top_r() const { return R.empty() ? inf() : R.top() + add_r; }
    T pop_l() {
        T a = top_l();
        L.pop();
        return a;
    }
    T pop_r() {
        T a = top_r();
        R.pop();
        return a;
    }

   public:
    /**
     * @brief 傾きが変わらない側の端を表す値 std::numeric_limits<T>::max() / 4
     */
    static constexpr T inf() { return std::numeric_limits<T>::max() / 4; }

    /**
     * @brief f の最小値
     */
    T min() const { return min_f; }
    /**
     * @brief f が最小値をとる区間 [lower, upper]。端がなければ -inf()、inf()
     */
    std::pair<T, T> argmin() const { return {top_l(), top_r()}; }
    /**
     * @brief f に定数 a を足す
     */
    void add_constant(T a) { min_f += a; }
    /**
     * @brief f に max(0, x - a) を足す O(log n)
     */
    void add_x_minus_a(T a) {
        min_f += std::max(T(0), top_l() - a);
        push_l(a);
        push_r(pop_l());
    }
    /**
     * @brief f に max(0, a - x) を足す O(log n)
     */
    void add_a_minus_x(T a) {
        min_f += std::max(T(0), a - top_r());
        push_r(a);
        push_l(pop_r());
    }
    /**
     * @brief f に |x - a| を足す O(log n)
     */
    void add_abs(T a) {
        add_x_minus_a(a);
        add_a_minus_x(a);
    }
    /**
     * @brief f(x) を min_{y <= x} f(y) にする (右側を平らにする) O(1)
     */
    void prefix_min() { R = {}; }
    /**
     * @brief f(x) を min_{y >= x} f(y) にする (左側を平らにする) O(1)
     */
    void suffix_min() { L = {}; }
    /**
     * @brief f(x) を min_{x - b <= y <= x - a} f(y) にする (スライド最小値) O(1)
     *
     * 最小値をとる区間より左を a、右を b だけ右にずらす。a <= b であること
     */
    void shift(T a, T b) {
        assert(a <= b);
        add_l += a, add_r += b;
    }
    /**
     * @brief f(x) を f(x - a) にする (a だけ右にずらす) O(1)
     */
    void shift(T a) { shift(a, a); }
    /**
     * @brief f(x) の値 O(n log n)
     */
    T get(T x) const {
        T res = min_f;
        for (auto l = L; !l.empty(); l.pop()) res += std::max(T(0), l.top() + add_l - x);
        for (auto r = R; !r.empty(); r.pop()) res += std::max(T(0), x - (r.top() + add_r));
        return res;
    }
};
