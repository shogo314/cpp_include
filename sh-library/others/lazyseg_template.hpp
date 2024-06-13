#pragma once
#include <optional>
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

template <typename T>
struct RangeUpdateRangeSum {
    using S = T;
    S op(const S& a, const S& b) const {
        return a + b;
    }
    S e() const {
        return {};
    }
    using F = std::optional<T>;
    S mapping(const F& f, const S& x, int sz) const {
        return f.has_value() ? f.value() * sz : x;
    }
    F composition(const F& f, const F& g) const {
        return f.has_value() ? f : g;
    }
    F id() const {
        return {};
    };
};

/**
 * @brief f(x)=ax+b の形の1次関数
 * f_{r-1}(f_{r-2}(...f_l(x))) を求める
 * @tparam MODINT 3以上の素数
 */
template <typename MODINT>
struct RangeUpdateRangeComposite {
    using S = std::pair<MODINT, MODINT>;
    S op(const S& l, const S& r) const {
        return {r.first * l.first, r.first * l.second + r.second};
    }
    S e() const {
        return {1, 0};
    }
    using F = std::optional<std::pair<MODINT, MODINT>>;
    S mapping(const F& f, const S& x, int sz) const {
        if (f.has_value()) {
            auto [pf, ps] = f.value();
            MODINT pw = pf.pow(sz);
            return {pw, (pw - 1) / (pf - 1) * ps};
        } else {
            return x;
        }
    }
    F composition(const F& f, const F& g) const {
        return f.has_value() ? f : g;
    }
    F id() const {
        return {};
    };
};
