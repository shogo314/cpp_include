#pragma once

/**
 * @file beats-acted-monoid.hpp
 * @brief Segment Tree Beats 用の作用素付きモノイド
 *
 * 作用素付きモノイド (algebra/acted-monoid.hpp) と同じメンバを持つが、
 * mapping は std::pair<bool, S> (作用に失敗したか, 作用後の値) を返す
 * data_structures/segtree-beats.hpp に渡して使う
 */

#include <algorithm>
#include <limits>
#include <utility>

#include "monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/algebra/beats-acted-monoid.hpp

namespace beats_acted_monoid {

/**
 * @brief 区間 chmin、区間和・最大値
 *
 * 作用素は chmin する値 (x <- min(x, f))
 * S(x) で値 x の1要素から作り、sum, max1 で区間和・最大値を取得する
 * 空の区間では max1 = numeric_limits<T>::lowest()
 * ChminChmaxAddSum より軽い
 *
 * @tparam T 値の型
 */
template <typename T>
struct ChminSum {
    static constexpr T lowest = std::numeric_limits<T>::lowest();

    struct S {
        T sum;
        T max1, max2;       // 最大値、2番目の最大値 (なければ lowest)
        long long max_cnt;  // 最大値の個数
        constexpr S() : sum(0), max1(lowest), max2(lowest), max_cnt(0) {}
        constexpr S(const T& x) : sum(x), max1(x), max2(lowest), max_cnt(1) {}
    };
    using F = T;

    static constexpr S op(const S& a, const S& b) {
        S res;
        res.sum = a.sum + b.sum;
        if (a.max1 == b.max1) {
            res.max1 = a.max1;
            res.max_cnt = a.max_cnt + b.max_cnt;
            res.max2 = std::max(a.max2, b.max2);
        } else if (a.max1 > b.max1) {
            res.max1 = a.max1;
            res.max_cnt = a.max_cnt;
            res.max2 = std::max(a.max2, b.max1);
        } else {
            res.max1 = b.max1;
            res.max_cnt = b.max_cnt;
            res.max2 = std::max(a.max1, b.max2);
        }
        return res;
    }
    static constexpr S e() { return S(); }
    static constexpr std::pair<bool, S> mapping(const F& f, const S& x) {
        if (x.max1 <= f) return {false, x};
        // 最大値だけが変わる (値が 1 種類なら f = lowest でも成功させる)
        if (x.max2 < f || x.max2 == lowest) {
            S res = x;
            res.sum -= (x.max1 - f) * T(x.max_cnt);
            res.max1 = f;
            return {false, res};
        }
        return {true, x};
    }
    static constexpr F composition(const F& f, const F& g) { return std::min(f, g); }
    static constexpr F id() { return std::numeric_limits<T>::max(); }
};

/**
 * @brief 区間 chmax、区間和・最小値
 *
 * 作用素は chmax する値 (x <- max(x, f))
 * S(x) で値 x の1要素から作り、sum, min1 で区間和・最小値を取得する
 * 空の区間では min1 = numeric_limits<T>::max()
 * ChminChmaxAddSum より軽い
 *
 * @tparam T 値の型
 */
template <typename T>
struct ChmaxSum {
    static constexpr T highest = std::numeric_limits<T>::max();

    struct S {
        T sum;
        T min1, min2;       // 最小値、2番目の最小値 (なければ highest)
        long long min_cnt;  // 最小値の個数
        constexpr S() : sum(0), min1(highest), min2(highest), min_cnt(0) {}
        constexpr S(const T& x) : sum(x), min1(x), min2(highest), min_cnt(1) {}
    };
    using F = T;

    static constexpr S op(const S& a, const S& b) {
        S res;
        res.sum = a.sum + b.sum;
        if (a.min1 == b.min1) {
            res.min1 = a.min1;
            res.min_cnt = a.min_cnt + b.min_cnt;
            res.min2 = std::min(a.min2, b.min2);
        } else if (a.min1 < b.min1) {
            res.min1 = a.min1;
            res.min_cnt = a.min_cnt;
            res.min2 = std::min(a.min2, b.min1);
        } else {
            res.min1 = b.min1;
            res.min_cnt = b.min_cnt;
            res.min2 = std::min(a.min1, b.min2);
        }
        return res;
    }
    static constexpr S e() { return S(); }
    static constexpr std::pair<bool, S> mapping(const F& f, const S& x) {
        if (x.min1 >= f) return {false, x};
        // 最小値だけが変わる (値が 1 種類なら f = highest でも成功させる)
        if (x.min2 > f || x.min2 == highest) {
            S res = x;
            res.sum += (f - x.min1) * T(x.min_cnt);
            res.min1 = f;
            return {false, res};
        }
        return {true, x};
    }
    static constexpr F composition(const F& f, const F& g) { return std::max(f, g); }
    static constexpr F id() { return std::numeric_limits<T>::lowest(); }
};

/**
 * @brief 区間 chmin・chmax・加算・代入、区間和・最大値・最小値
 *
 * 作用素 F = ClampAdd<T>{lb, ub, add} は x <- clamp(x + add, lb, ub) を表す (lb <= ub)
 * chmin(b), chmax(b), add(b), update(b) で作る
 * S(x) で値 x の1要素から作り、sum, max1, min1 で区間和・最大値・最小値を取得する
 * 空の区間では max1 = numeric_limits<T>::lowest(), min1 = numeric_limits<T>::max()
 *
 * @tparam T 値の型 (符号付き整数を想定する)
 */
template <typename T>
struct ChminChmaxAddSum {
    static constexpr T lowest = std::numeric_limits<T>::lowest();
    static constexpr T highest = std::numeric_limits<T>::max();

    struct S {
        T sum;
        T max1, max2;                // 最大値、2番目の最大値 (なければ lowest)
        T min1, min2;                // 最小値、2番目の最小値 (なければ highest)
        long long max_cnt, min_cnt;  // 最大値・最小値の個数
        constexpr S() : sum(0), max1(lowest), max2(lowest), min1(highest), min2(highest), max_cnt(0), min_cnt(0) {}
        constexpr S(const T& x) : sum(x), max1(x), max2(lowest), min1(x), min2(highest), max_cnt(1), min_cnt(1) {}
    };
    using F = ClampAdd<T>;

    static constexpr F chmin(const T& b) { return {lowest, b, T(0)}; }
    static constexpr F chmax(const T& b) { return {b, highest, T(0)}; }
    static constexpr F add(const T& b) { return {lowest, highest, b}; }
    static constexpr F update(const T& b) { return {b, b, T(0)}; }

    static constexpr S op(const S& a, const S& b) {
        S res;
        res.sum = a.sum + b.sum;
        if (a.max1 == b.max1) {
            res.max1 = a.max1;
            res.max_cnt = a.max_cnt + b.max_cnt;
            res.max2 = std::max(a.max2, b.max2);
        } else if (a.max1 > b.max1) {
            res.max1 = a.max1;
            res.max_cnt = a.max_cnt;
            res.max2 = std::max(a.max2, b.max1);
        } else {
            res.max1 = b.max1;
            res.max_cnt = b.max_cnt;
            res.max2 = std::max(a.max1, b.max2);
        }
        if (a.min1 == b.min1) {
            res.min1 = a.min1;
            res.min_cnt = a.min_cnt + b.min_cnt;
            res.min2 = std::min(a.min2, b.min2);
        } else if (a.min1 < b.min1) {
            res.min1 = a.min1;
            res.min_cnt = a.min_cnt;
            res.min2 = std::min(a.min2, b.min1);
        } else {
            res.min1 = b.min1;
            res.min_cnt = b.min_cnt;
            res.min2 = std::min(a.min1, b.min2);
        }
        return res;
    }
    static constexpr S e() { return S(); }
    static constexpr std::pair<bool, S> mapping(const F& f, const S& x, long long sz) {
        if (x.max_cnt == 0) return {false, x};
        S res = x;
        // 加算 (番兵には足さない)
        res.sum += f.add * T(sz);
        res.max1 += f.add;
        res.min1 += f.add;
        if (res.max2 != lowest) res.max2 += f.add;
        if (res.min2 != highest) res.min2 += f.add;
        // 値が 1 種類
        if (res.max1 == res.min1) {
            T v = std::clamp(res.max1, f.lb, f.ub);
            res.max1 = res.min1 = v;
            res.sum = v * T(sz);
            return {false, res};
        }
        // 値が 2 種類
        if (res.max1 == res.min2) {
            T hi = std::clamp(res.max1, f.lb, f.ub), lo = std::clamp(res.min1, f.lb, f.ub);
            if (hi == lo) {
                res.max1 = res.min1 = hi;
                res.max2 = lowest;
                res.min2 = highest;
                res.max_cnt = res.min_cnt = sz;
                res.sum = hi * T(sz);
            } else {
                res.max1 = res.min2 = hi;
                res.min1 = res.max2 = lo;
                res.sum = hi * T(res.max_cnt) + lo * T(res.min_cnt);
            }
            return {false, res};
        }
        // 値が 3 種類以上: 最大値と最小値だけが変わる場合以外は失敗
        if (f.ub <= res.max2 || f.lb >= res.min2) return {true, res};
        if (f.ub < res.max1) {
            res.sum -= (res.max1 - f.ub) * T(res.max_cnt);
            res.max1 = f.ub;
        }
        if (f.lb > res.min1) {
            res.sum += (f.lb - res.min1) * T(res.min_cnt);
            res.min1 = f.lb;
        }
        return {false, res};
    }
    // g を適用した後に f を適用する
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampAddComposite<T>::op(f, g); }
    static constexpr F id() { return {lowest, highest, T(0)}; }
};

}  // namespace beats_acted_monoid
