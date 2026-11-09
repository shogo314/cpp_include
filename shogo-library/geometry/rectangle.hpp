#pragma once

/**
 * @file rectangle.hpp
 * @brief 軸に平行な長方形
 */

#include <cassert>
#include <istream>
#include <type_traits>

#include "vector.hpp"
// https://github.com/shogo314/shogo-library/blob/27998142f20ef4e094add3b53cc9c070db4ddbf8/src/geometry/rectangle.hpp

/**
 * @brief 半開領域 [left, right) × [bottom, top)。幅・高さ 0 も許す
 * @tparam T 座標の型。省略したら long long
 */
template <typename T = long long>
struct Rectangle {
    T left{}, bottom{}, right{}, top{};

    constexpr Rectangle() = default;
    constexpr Rectangle(T left, T bottom, T right, T top) : left(left), bottom(bottom), right(right), top(top) {
        assert(left <= right && bottom <= top);
    }
    constexpr T width() const { return right - left; }
    constexpr T height() const { return top - bottom; }
    constexpr T area() const { return width() * height(); }
    constexpr bool empty() const { return left == right || bottom == top; }
    /** @brief 左辺・下辺を含み、右辺・上辺を含まない包含判定 */
    constexpr bool contains(T x, T y) const {
        return left <= x && x < right && bottom <= y && y < top;
    }
    constexpr bool contains(const Vector<T, 2>& p) const { return contains(p[0], p[1]); }
    /** @brief 4 辺の座標がすべて等しいか (空の長方形でも座標を比較する) */
    constexpr bool operator==(const Rectangle& other) const {
        return left == other.left && bottom == other.bottom && right == other.right && top == other.top;
    }
    constexpr bool operator!=(const Rectangle& other) const { return !(*this == other); }

    /** @brief left bottom right top の順に読み込む */
    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, Rectangle& rhs) {
        if (is >> rhs.left >> rhs.bottom >> rhs.right >> rhs.top) {
            assert(rhs.left <= rhs.right && rhs.bottom <= rhs.top);
        }
        return is;
    }
};

template <typename A, typename B, typename C, typename D>
Rectangle(A, B, C, D) -> Rectangle<std::common_type_t<A, B, C, D>>;
