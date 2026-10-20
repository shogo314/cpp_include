#pragma once

/**
 * @file project-selection.hpp
 * @brief 燃やす埋める (Project Selection Problem)
 */

#include <cassert>
#include <utility>
#include <vector>

#include "max-flow.hpp"
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/graph/project-selection.hpp

/**
 * @brief 燃やす埋める (Project Selection Problem)
 *
 * n 個の 0/1 変数 x_0, ..., x_{n-1} について、コストの和の最小値と、そのときの割り当てを最小カットで求める
 * 変数 v は、最小カットで始点側なら x_v = 0、終点側なら x_v = 1
 * 2 変数のコストは劣モジュラ (c01 + c10 >= c00 + c11) であること
 * 例:
 * ProjectSelection<long long> ps(n);
 * ps.add_cost(0, 3, 5);           // x_0 = 0 ならコスト 3、1 ならコスト 5
 * ps.add_cost_if(0, 1, 1, 0, 4);  // x_0 = 1 かつ x_1 = 0 ならコスト 4
 * long long c = ps.solve();
 * std::vector<bool> x = ps.assignment();
 *
 * @tparam Cost コストの型
 */
template <typename Cost = long long>
class ProjectSelection {
    long long n;
    long long aux = 0;  // 補助頂点の個数
    Cost constant = 0;
    std::vector<Cost> cost1;  // x_v = 1 のときの追加のコスト (cost0 を引いた差)
    // 辺 from -> to (from が 0 かつ to が 1 ならコスト cap)。inf なら容量は十分大きい値にする
    struct E {
        long long from, to;
        Cost cap;
        bool inf;
    };
    std::vector<E> es;
    std::vector<bool> result;

    // 頂点の番号: 変数 0, ..., n - 1、補助頂点 n, ..., n + aux - 1、始点 n + aux、終点 n + aux + 1
    void check_var(long long v) const { assert(0 <= v && v < n); }

   public:
    /**
     * @brief n 個の変数で、コストがすべて 0
     */
    explicit ProjectSelection(long long n = 0) : n(n), cost1(n, 0) {}

    /**
     * @brief 変数の個数
     */
    long long size() const { return n; }

    /**
     * @brief 定数のコスト c を足す
     */
    void add_constant(Cost c) { constant += c; }
    /**
     * @brief x_v = 0 ならコスト c0、x_v = 1 ならコスト c1 を足す (負でもよい)
     */
    void add_cost(long long v, Cost c0, Cost c1) {
        check_var(v);
        constant += c0;
        cost1[v] += c1 - c0;
    }
    /**
     * @brief x_v = a ならコスト c を足す (負でもよい)
     */
    void add_cost_if(long long v, bool a, Cost c) {
        if (a) {
            add_cost(v, 0, c);
        } else {
            add_cost(v, c, 0);
        }
    }
    /**
     * @brief (x_u, x_v) が (0, 0)、(0, 1)、(1, 0)、(1, 1) のときのコスト c00、c01、c10、c11 を足す
     * @pre 劣モジュラ: c01 + c10 >= c00 + c11
     */
    void add_cost(long long u, long long v, Cost c00, Cost c01, Cost c10, Cost c11) {
        check_var(u), check_var(v);
        assert(u != v);
        assert(c01 + c10 >= c00 + c11 && "ProjectSelection: not submodular");
        // c00 + (c10 - c00) x_u + (c11 - c10) x_v + (c01 + c10 - c00 - c11) [x_u = 0 かつ x_v = 1]
        constant += c00;
        cost1[u] += c10 - c00;
        cost1[v] += c11 - c10;
        Cost w = c01 + c10 - c00 - c11;
        if (w > 0) es.push_back({u, v, w, false});
    }
    /**
     * @brief x_u = a かつ x_v = b ならコスト c を足す
     * @pre a != b なら c >= 0、a == b なら c <= 0 (劣モジュラになる場合)
     */
    void add_cost_if(long long u, bool a, long long v, bool b, Cost c) {
        Cost cs[2][2] = {{0, 0}, {0, 0}};
        cs[a][b] = c;
        add_cost(u, v, cs[0][0], cs[0][1], cs[1][0], cs[1][1]);
    }
    /**
     * @brief x_u = a かつ x_v = !a となる割り当てを禁止する (コストを十分大きくする)
     */
    void forbid(long long u, bool a, long long v) {
        check_var(u), check_var(v);
        if (a) {
            es.push_back({v, u, 0, true});
        } else {
            es.push_back({u, v, 0, true});
        }
    }
    /**
     * @brief vs の変数がすべて a なら利得 g を得る (コスト -g を足す)
     * 補助頂点を 1 つ使う
     * @pre g >= 0
     */
    void add_gain_if_all(const std::vector<long long>& vs, bool a, Cost g) {
        assert(g >= 0);
        for (long long v : vs) check_var(v);
        long long y = n + aux++;
        constant -= g;
        if (a) {
            // y = 0 ならコスト g、y = 1 は vs がすべて 1 のときだけ
            es.push_back({y, -1, g, false});
            for (long long v : vs) es.push_back({v, y, 0, true});
        } else {
            // y = 1 ならコスト g、y = 0 は vs がすべて 0 のときだけ
            es.push_back({-1, y, g, false});
            for (long long v : vs) es.push_back({y, v, 0, true});
        }
    }

    /**
     * @brief コストの和の最小値
     */
    Cost solve() {
        const long long s = n + aux, t = s + 1;
        MaxFlow<Cost> mf(t + 1);
        // 十分大きい容量: 有限の容量の和 + 1
        Cost inf = 1;
        for (long long v = 0; v < n; v++) inf += cost1[v] > 0 ? cost1[v] : -cost1[v];
        for (const auto& e : es) {
            if (!e.inf) inf += e.cap;
        }
        Cost base = constant;
        for (long long v = 0; v < n; v++) {
            if (cost1[v] > 0) {
                // x_v = 1 (終点側) ならコスト: 始点から v への辺
                mf.add_edge(s, v, cost1[v]);
            } else if (cost1[v] < 0) {
                // x_v = 0 なら -cost1[v] 多くかかると考え、定数から引く
                base += cost1[v];
                mf.add_edge(v, t, -cost1[v]);
            }
        }
        for (const auto& e : es) {
            // 補助頂点への辺の -1 は、始点 (from) か終点 (to)
            long long from = e.from == -1 ? s : e.from, to = e.to == -1 ? t : e.to;
            mf.add_edge(from, to, e.inf ? inf : e.cap);
        }
        Cost f = mf.flow(s, t);
        std::vector<bool> reach = mf.min_cut(s);
        result.assign(n, false);
        for (long long v = 0; v < n; v++) result[v] = !reach[v];
        return base + f;
    }
    /**
     * @brief 直前の solve() で最小値をとる割り当て (x_v が 1 なら true)
     */
    const std::vector<bool>& assignment() const { return result; }
};
