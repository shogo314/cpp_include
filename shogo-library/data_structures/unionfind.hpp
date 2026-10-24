#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/unionfind.hpp

/**
 * @file unionfind.hpp
 * @brief UnionFind
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace internal_unionfind {
/**
 * @brief 各要素の代表元から、連結成分ごとの要素のリストを作る
 * @param leader 要素 a の代表元を返す関数
 */
template <typename F>
std::vector<std::vector<long long>> groups(long long n, F leader) {
    std::vector<long long> leader_buf(n), group_size(n);
    for (long long i = 0; i < n; i++) {
        leader_buf[i] = leader(i);
        group_size[leader_buf[i]]++;
    }
    std::vector<std::vector<long long>> result(n);
    for (long long i = 0; i < n; i++) {
        result[i].reserve(group_size[i]);
    }
    for (long long i = 0; i < n; i++) {
        result[leader_buf[i]].push_back(i);
    }
    result.erase(std::remove_if(result.begin(), result.end(), [&](const std::vector<long long>& v) { return v.empty(); }), result.end());
    return result;
}
}  // namespace internal_unionfind

/**
 * @brief 無向グラフに対して「辺の追加」、「2頂点が連結かの判定」をする
 */
class UnionFind {
   protected:
    long long _n;
    // 負ならサイズ、非負なら親 (メモリを抑えるため int で持つ)
    std::vector<int> parent_or_size;

   public:
    /**
     * @brief merge の戻り値
     *
     * if (uf.merge(a, b)) で新しく統合したかを判定でき、
     * auto [r, merged] = uf.merge(a, b); で代表元も受け取れる
     */
    struct MergeResult {
        long long leader;  //!< 統合後の代表元
        bool merged;       //!< 新しく統合したか (もともと同じ連結成分なら false)
        explicit operator bool() const { return merged; }
    };

    UnionFind() : _n(0) {}
    explicit UnionFind(long long n) : _n(n), parent_or_size(n, -1) {}

    /**
     * @brief 辺(a,b)を足す
     * @return 統合後の代表元と、新しく統合したか
     */
    MergeResult merge(long long a, long long b) {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        long long x = leader(a), y = leader(b);
        if (x == y) return {x, false};
        if (-parent_or_size[x] < -parent_or_size[y]) std::swap(x, y);
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = static_cast<int>(x);
        return {x, true};
    }
    /**
     * @brief 頂点a,bが連結かどうか
     */
    bool same(long long a, long long b) {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        return leader(a) == leader(b);
    }
    /**
     * @brief 頂点aの属する連結成分の代表元
     */
    long long leader(long long a) {
        assert(0 <= a && a < _n);
        if (parent_or_size[a] < 0) return a;
        int x = static_cast<int>(a);
        while (parent_or_size[x] >= 0) {
            x = parent_or_size[x];
        }
        while (parent_or_size[a] >= 0) {
            int t = parent_or_size[a];
            parent_or_size[a] = x;
            a = t;
        }
        return x;
    }
    /**
     * @brief 頂点aの属する連結成分のサイズ
     */
    long long size(long long a) {
        assert(0 <= a && a < _n);
        return -parent_or_size[leader(a)];
    }
    /**
     * @brief グラフを連結成分に分け、その情報を返す
     * @return 「一つの連結成分の頂点番号のリスト」のリスト
     */
    std::vector<std::vector<long long>> groups() {
        return internal_unionfind::groups(_n, [&](long long a) { return leader(a); });
    }
};
