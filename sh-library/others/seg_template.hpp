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

struct Parenthesis {
    struct S {
        long long left;
        long long right;
        S(char c = 0) {
            if (c == '(') {
                left = 0;
                right = 1;
            } else if (c == ')') {
                left = 1;
                right = 0;
            } else {
                left = right = 0;
            }
        }
        S(long long left_, long long right_) : left(left_), right(right_) {}
        bool is_valid() const {
            return left == 0 && right == 0;
        }
    };
    S op(const S& l, const S& r) {
        if (l.right >= r.left) {
            return S(l.left, l.right - r.left + r.right);
        } else {
            return S(l.left + r.left - l.right, r.right);
        }
    }
    S e() {
        return S();
    }
};
