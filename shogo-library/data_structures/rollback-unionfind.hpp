#pragma once

/**
 * @file rollback-unionfind.hpp
 * @brief 巻き戻しのできる UnionFind
 */

#include <cassert>
#include <utility>
#include <vector>

#include "unionfind.hpp"
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/data_structures/rollback-unionfind.hpp

/**
 * @brief union by size を使い、経路圧縮をせずに更新履歴を残す UnionFind
 */
class RollbackUnionFind : private UnionFind {
    struct Change {
        int x, y, size_x, size_y;
    };
    std::vector<Change> history;

   public:
    using MergeResult = UnionFind::MergeResult;

    explicit RollbackUnionFind(long long n = 0) : UnionFind(n) { assert(n >= 0); }

    long long leader(long long a) const {
        assert(0 <= a && a < _n);
        while (parent_or_size[a] >= 0) a = parent_or_size[a];
        return a;
    }
    bool same(long long a, long long b) const { return leader(a) == leader(b); }
    long long size(long long a) const { return -parent_or_size[leader(a)]; }

    MergeResult merge(long long a, long long b) {
        long long x = leader(a), y = leader(b);
        if (x == y) return {x, false};
        if (-parent_or_size[x] < -parent_or_size[y]) std::swap(x, y);
        history.push_back({static_cast<int>(x), static_cast<int>(y), parent_or_size[x], parent_or_size[y]});
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = static_cast<int>(x);
        return {x, true};
    }

    /** @brief 現在の履歴の位置。同じ成分を merge しても増えない */
    long long snapshot() const { return static_cast<long long>(history.size()); }
    /** @brief 現在の履歴上の、snapshot で取得した位置まで巻き戻す */
    void rollback(long long t) {
        assert(0 <= t && t <= snapshot());
        while (snapshot() > t) {
            const auto c = history.back();
            history.pop_back();
            parent_or_size[c.x] = c.size_x;
            parent_or_size[c.y] = c.size_y;
        }
    }
    std::vector<std::vector<long long>> groups() const {
        return internal_unionfind::groups(_n, [&](long long a) { return leader(a); });
    }
};
