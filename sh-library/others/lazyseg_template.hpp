#pragma once
/**
 * @brief https://github.com/rainbou-kpr/library/blob/main/cpp/lazy-segtree.hpp
 * TemplateLazySegTree に乗せるための構造体
 */

/**
 * @brief 区間に対して x <- a * x + b を作用
 * 区間sumを求める
 */
template <typename T>
struct RangeAffineRangeSum {
    using S = T;
    S op(const S& a, const S& b) const {
        return a + b;
    }
    S e() const {
        return 0;
    }
    using F = std::pair<T, T>;
    S mapping(const F& f, const S& x, int sz) const {
        return f.first * x + f.second * T(sz);
    }
    F composition(const F& f, const F& g) const {
        return {f.first * g.first, f.first * g.second + f.second};
    }
    F id() const {
        return {1, 0};
    };
};
