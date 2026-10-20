#pragma once

/**
 * @file sliding-window-aggregation.hpp
 * @brief Sliding Window Aggregation (モノイドを乗せたキュー) とスライド最小値
 */

#include <cassert>
#include <deque>
#include <functional>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/data_structures/sliding-window-aggregation.hpp

/**
 * @brief Sliding Window Aggregation (モノイドを乗せたキュー)
 *
 * 末尾への追加、先頭の削除、全体の積を償却 O(1) で行う。モノイドは可換でなくてもよい
 * 例:
 * SlidingWindowAggregation<monoid::Reverse<monoid::Composite<mint>>> swag;  // 先頭から順に適用
 * swag.push(f); swag.pop(); swag.prod();
 *
 * @tparam M モノイド (algebra/monoid.hpp)
 */
template <typename M>
class SlidingWindowAggregation {
   public:
    using S = typename M::S;

   private:
    M m;
    // キューの前半をスタック front_vals (末尾がキューの先頭) で、後半を back_vals (末尾がキューの末尾) で持つ
    std::vector<S> front_vals, front_prod;  // front_prod[i] は front_vals[i], front_vals[i - 1], ..., front_vals[0] の積
    std::vector<S> back_vals;
    S back_prod;  // back_vals の先頭からの積

    S op(const S& x, const S& y) const { return m.op(x, y); }
    S e() const { return m.e(); }

   public:
    explicit SlidingWindowAggregation(M m = M()) : m(std::move(m)), back_prod(this->m.e()) {}

    /**
     * @brief 要素数
     */
    long long size() const { return front_vals.size() + back_vals.size(); }
    bool empty() const { return size() == 0; }

    /**
     * @brief 末尾に x を追加する O(1)
     */
    void push(const S& x) {
        back_vals.push_back(x);
        back_prod = op(back_prod, x);
    }
    /**
     * @brief 先頭の要素を削除する 償却 O(1)
     */
    void pop() {
        assert(!empty());
        if (front_vals.empty()) {
            // 後半をすべて前半へ移す
            while (!back_vals.empty()) {
                S x = std::move(back_vals.back());
                back_vals.pop_back();
                front_prod.push_back(front_vals.empty() ? x : op(x, front_prod.back()));
                front_vals.push_back(std::move(x));
            }
            back_prod = e();
        }
        front_vals.pop_back();
        front_prod.pop_back();
    }
    /**
     * @brief 先頭の要素 O(1)
     */
    const S& front() const {
        assert(!empty());
        return front_vals.empty() ? back_vals.front() : front_vals.back();
    }
    /**
     * @brief 先頭から末尾までの積 (空なら単位元) O(1)
     */
    S prod() const {
        if (front_vals.empty()) return back_prod;
        return op(front_prod.back(), back_prod);
    }
    /**
     * @brief すべての要素を削除する
     */
    void clear() {
        front_vals.clear();
        front_prod.clear();
        back_vals.clear();
        back_prod = e();
    }
};

template <typename M>
SlidingWindowAggregation(M) -> SlidingWindowAggregation<M>;

/**
 * @brief 長さ k の各区間 [i, i + k) の最小値を並べたもの (長さ n - k + 1) O(n)
 *
 * 単調な deque を使う
 * @param comp 大小の比較。std::greater<T>() を渡すと最大値になる
 */
template <typename T, typename Compare = std::less<T>>
std::vector<T> sliding_window_min(const std::vector<T>& a, long long k, Compare comp = Compare()) {
    long long n = a.size();
    assert(1 <= k && k <= n);
    std::vector<T> res;
    res.reserve(n - k + 1);
    // 窓の中で、後ろにそれ以下の値がない添字を、値の昇順に持つ
    std::deque<long long> dq;
    for (long long i = 0; i < n; i++) {
        while (!dq.empty() && !comp(a[dq.back()], a[i])) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();
        if (i >= k - 1) res.push_back(a[dq.front()]);
    }
    return res;
}

/**
 * @brief 長さ k の各区間 [i, i + k) の最大値を並べたもの (長さ n - k + 1) O(n)
 */
template <typename T>
std::vector<T> sliding_window_max(const std::vector<T>& a, long long k) {
    return sliding_window_min(a, k, std::greater<T>());
}
