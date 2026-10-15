#pragma once

/**
 * @file persistent-unionfind.hpp
 * @brief 永続UnionFind
 */

#include <cassert>
#include <utility>
#include <vector>

#include "persistent-array.hpp"
#include "unionfind.hpp"
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/data_structures/persistent-unionfind.hpp

/**
 * @brief 永続UnionFind
 *
 * 辺を追加すると新しい版を返し、元の版は変わらない。union by size だけを行い、経路圧縮はしない
 * 例:
 * PersistentUnionFind uf0(n);
 * auto uf1 = uf0.merge(0, 1);  // uf0 はそのまま
 * uf1.same(0, 1);              // true
 */
class PersistentUnionFind {
    long long _n;
    // 負ならサイズ、非負なら親 (メモリを抑えるため int で持つ)
    PersistentArray<int> parent_or_size;

    PersistentUnionFind(long long n, PersistentArray<int> parent_or_size) : _n(n), parent_or_size(std::move(parent_or_size)) {}

   public:
    PersistentUnionFind() : _n(0) {}
    explicit PersistentUnionFind(long long n) : _n(n), parent_or_size(n, -1) {}

    /**
     * @brief 辺(a,b)を足した新しい版を返す
     * @return PersistentUnionFind 新しい版 (自身は変わらない。もともと連結なら同じ版)
     */
    PersistentUnionFind merge(long long a, long long b) const {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        long long x = leader(a), y = leader(b);
        if (x == y) return *this;
        int sx = parent_or_size[x], sy = parent_or_size[y];
        if (-sx < -sy) std::swap(x, y);
        return PersistentUnionFind(_n, parent_or_size.set(x, sx + sy).set(y, int(x)));
    }
    /**
     * @brief 頂点a,bが連結かどうか
     */
    bool same(long long a, long long b) const {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        return leader(a) == leader(b);
    }
    /**
     * @brief 頂点aの属する連結成分の代表元
     */
    long long leader(long long a) const {
        assert(0 <= a && a < _n);
        while (true) {
            int p = parent_or_size[a];
            if (p < 0) return a;
            a = p;
        }
    }
    /**
     * @brief 頂点aの属する連結成分のサイズ
     */
    long long size(long long a) const {
        assert(0 <= a && a < _n);
        return -parent_or_size[leader(a)];
    }
    /**
     * @brief グラフを連結成分に分け、その情報を返す
     * @return 「一つの連結成分の頂点番号のリスト」のリスト
     */
    std::vector<std::vector<long long>> groups() const {
        return internal_unionfind::groups(_n, [&](long long a) { return leader(a); });
    }
};
