#pragma once

/**
 * @file potentialized-unionfind.hpp
 * @brief ポテンシャル付きUnionFind
 */
#include <cassert>
#include <stack>
#include <utility>
#include <vector>

#include "../algebra/group.hpp"
#include "unionfind.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/potentialized-unionfind.hpp

/**
 * @brief ポテンシャル付きUnionFind
 * @tparam G ポテンシャルの群 (algebra/group.hpp) 非可換でもよい
 */
template <typename G>
class PotentializedUnionFind {
   public:
    using S = typename G::S;
    using MergeResult = UnionFind::MergeResult;

   private:
    G g;
    long long _n;
    // 負ならサイズ、非負なら親 (メモリを抑えるため int で持つ)
    std::vector<int> parent_or_size;
    // 重み
    std::vector<S> diff_weight;

    S op(const S& a, const S& b) const { return g.op(a, b); }
    S e() const { return g.e(); }
    S inv(const S& a) const { return g.inv(a); }

   public:
    /**
     * @param n 要素数
     * @param g ポテンシャルの群
     */
    explicit PotentializedUnionFind(long long n = 0, G g = G()) : g(std::move(g)), _n(n), parent_or_size(n, -1), diff_weight(n, e()) {}

    /**
     * @brief 頂点aの属する連結成分の代表元
     */
    long long leader(long long a) {
        assert(0 <= a && a < _n);
        if (parent_or_size[a] < 0) return a;
        std::stack<int> stk;
        stk.push(static_cast<int>(a));
        while (parent_or_size[stk.top()] >= 0) {
            stk.push(parent_or_size[stk.top()]);
        }
        const int root = stk.top();
        stk.pop();
        stk.pop();
        while (!stk.empty()) {
            diff_weight[stk.top()] = op(diff_weight[parent_or_size[stk.top()]], diff_weight[stk.top()]);
            parent_or_size[stk.top()] = root;
            stk.pop();
        }
        return root;
    }

    /**
     * @brief a のグループと b のグループを統合する
     * @param w (b のポテンシャル) - (a のポテンシャル)
     * @return 統合後の代表元と、新しく統合したか
     * @note すでに同じ連結成分なら何もしない (w が矛盾していても検出しない)。
     *       矛盾を調べるには、merged が false のときに diff(a, b) == w かを確かめる
     */
    MergeResult merge(long long a, long long b, S w) {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        w = op(weight(a), w);
        w = op(w, inv(weight(b)));
        long long x = leader(a), y = leader(b);
        if (x == y) return {x, false};
        if (-parent_or_size[x] < -parent_or_size[y]) {
            std::swap(x, y);
            w = inv(w);
        }
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = static_cast<int>(x);
        diff_weight[y] = w;
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
     * @brief (b のポテンシャル) - (a のポテンシャル)
     */
    S diff(long long a, long long b) {
        assert(same(a, b));
        return op(inv(weight(a)), weight(b));
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

   private:
    S weight(long long a) {
        leader(a);
        return diff_weight[a];
    }
};

template <typename G>
PotentializedUnionFind(long long, G) -> PotentializedUnionFind<G>;
