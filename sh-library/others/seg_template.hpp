#pragma once
/**
 * @brief https://github.com/rainbou-kpr/library/blob/main/cpp/segtree.hpp
 * TemplateSegTree に乗せるための構造体
 */

/**
 * @brief f(x)=ax+b の形の1次関数
 * f_{r-1}(f_{r-2}(...f_l(x))) を求める
 */
template <typename T>
struct RangeComposite {
    using S = std::pair<T, T>;
    S op(const S& l, const S& r) {
        return {r.first * l.first, r.first * l.second + r.second};
    }
    S e() {
        return {1, 0};
    }
};
