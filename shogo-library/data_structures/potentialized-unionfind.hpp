#pragma once

/**
 * @file potentialized-unionfind.hpp
 * @brief ポテンシャル付きUnionFind
 */
#include <algorithm>
#include <cassert>
#include <functional>
#include <stack>
#include <utility>
#include <vector>
#include "../algebra/group.hpp"

/**
 * @brief ポテンシャル付きUnionFind
 * @tparam G ポテンシャルの群 (algebra/group.hpp) 非可換でもよい
 */
template <typename G>
class PotentializedUnionFind {
   public:
    using S = typename G::S;

   private:
    G g;
    int _n;
    // 負ならサイズ、非負なら親
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
    explicit PotentializedUnionFind(int n = 0, G g = G()) : g(std::move(g)), _n(n), parent_or_size(n, -1), diff_weight(n, e()) {}

    /**
     * @brief 頂点aの属する連結成分の代表元
     */
    int leader(int a) {
        assert(0 <= a && a < _n);
        if (parent_or_size[a] < 0) return a;
        std::stack<int> stk;
        stk.push(a);
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
     * @return 連結したものの代表元
     */
    int merge(int a, int b, S w) {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        w = op(weight(a), w);
        w = op(w, inv(weight(b)));
        int x = leader(a), y = leader(b);
        if (x == y) return x;
        if (-parent_or_size[x] < -parent_or_size[y]) {
            std::swap(x, y);
            w = inv(w);
        }
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = x;
        diff_weight[y] = w;
        return x;
    }

    /**
     * @brief 頂点a,bが連結かどうか
     */
    bool same(int a, int b) {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        return leader(a) == leader(b);
    }

    /**
     * @brief (b のポテンシャル) - (a のポテンシャル)
     */
    S diff(int a, int b) {
        assert(same(a, b));
        return op(inv(weight(a)), weight(b));
    }

    /**
     * @brief 頂点aの属する連結成分のサイズ
     */
    int size(int a) {
        assert(0 <= a && a < _n);
        return -parent_or_size[leader(a)];
    }

    /**
     * @brief グラフを連結成分に分け、その情報を返す
     * @return 「一つの連結成分の頂点番号のリスト」のリスト
     */
    std::vector<std::vector<int>> groups() {
        std::vector<int> leader_buf(_n), group_size(_n);
        for (int i = 0; i < _n; i++) {
            leader_buf[i] = leader(i);
            group_size[leader_buf[i]]++;
        }
        std::vector<std::vector<int>> result(_n);
        for (int i = 0; i < _n; i++) {
            result[i].reserve(group_size[i]);
        }
        for (int i = 0; i < _n; i++) {
            result[leader_buf[i]].push_back(i);
        }
        result.erase(std::remove_if(result.begin(), result.end(), [&](const std::vector<int>& v) { return v.empty(); }), result.end());
        return result;
    }

   private:
    S weight(int a) {
        leader(a);
        return diff_weight[a];
    }
};

template <typename G>
PotentializedUnionFind(int, G) -> PotentializedUnionFind<G>;
