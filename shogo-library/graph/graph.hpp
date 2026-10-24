#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/graph/graph.hpp

/**
 * @file graph.hpp
 * @brief グラフ
 */

#include <cassert>
#include <iostream>
#include <vector>

/**
 * @brief グラフの汎用クラス
 *
 * @tparam Cost 辺のコストの型 省略したら long long (重みの和や未到達を表す max / 4 があふれにくいように)
 */
template <typename Cost = long long>
class Graph {
   public:
    /**
     * @brief 有向辺の構造体
     *
     * operator long long()を定義しているので、整数型にキャストすると勝手にdstになる
     * 例えば、
     * for (auto& e : g[v]) をすると、vから出る辺が列挙されるが、
     * for (long long dst : g[v]) とすると、vから出る辺の行き先が列挙される
     */
    struct Edge {
        long long src = -1;  //!< 始点
        long long dst = -1;  //!< 終点
        Cost cost = {};      //!< コスト
        long long id = -1;   //!< 辺の番号(追加された順、無向辺の場合はidが同じで方向が逆のものが2つ存在する)
        Edge() = default;
        Edge(long long src, long long dst, Cost cost = 1, long long id = -1) : src(src), dst(dst), cost(cost), id(id) {}
        operator long long() const { return dst; }
    };

    long long n;                       //!< 頂点数
    long long m;                       //!< 辺数
    std::vector<std::vector<Edge>> g;  //!< グラフの隣接リスト表現

    /**
     * @brief デフォルトコンストラクタ
     */
    Graph() : n(0), m(0), g(0) {}
    /**
     * @brief コンストラクタ
     * @param n 頂点数
     */
    explicit Graph(long long n) : n(n), m(0), g(n) {}
    /**
     * @brief 頂点数
     */
    long long size() const { return n; }
    /**
     * @brief 無向辺を追加する
     * @param u 始点
     * @param v 終点
     * @param w コスト 省略したら1
     */
    void add_edge(long long u, long long v, Cost w = 1) {
        assert(0 <= u && u < n && 0 <= v && v < n);
        g[u].push_back({u, v, w, m});
        g[v].push_back({v, u, w, m++});
    }
    /**
     * @brief 有向辺を追加する
     * @param u 始点
     * @param v 終点
     * @param w コスト 省略したら1
     */
    void add_directed_edge(long long u, long long v, Cost w = 1) {
        assert(0 <= u && u < n && 0 <= v && v < n);
        g[u].push_back({u, v, w, m++});
    }
    /**
     * @brief 辺の情報を標準入力から受け取って追加する
     * @param m 辺の数
     * @param padding 頂点番号を入力からいくつずらすか 省略したら-1
     * @param weighted 辺の重みが入力されるか 省略したらfalseとなり、重み1で辺が追加される
     * @param directed 有向グラフかどうか 省略したらfalse
     */
    void read(long long m, long long padding = -1, bool weighted = false, bool directed = false) {
        for (long long i = 0; i < m; i++) {
            long long u, v;
            std::cin >> u >> v;
            u += padding, v += padding;
            Cost c(1);
            if (weighted)
                std::cin >> c;
            if (directed)
                add_directed_edge(u, v, c);
            else
                add_edge(u, v, c);
        }
    }
    /**
     * @brief ある頂点から出る辺を列挙する
     * @param v 頂点番号
     * @return std::vector<Edge>& vから出る辺のリスト
     */
    std::vector<Edge>& operator[](long long v) {
        return g[v];
    }
    /**
     * @brief ある頂点から出る辺を列挙する
     * @param v 頂点番号
     * @return const std::vector<Edge>& vから出る辺のリスト
     */
    const std::vector<Edge>& operator[](long long v) const {
        return g[v];
    }
    /**
     * @brief 辺のリスト
     * @return std::vector<Edge> 辺のリスト(idの昇順)
     *
     * 無向辺は代表して1つだけ格納される
     */
    std::vector<Edge> edges() const {
        std::vector<Edge> res(m);
        for (long long i = 0; i < n; i++) {
            for (auto& e : g[i]) {
                res[e.id] = e;
            }
        }
        return res;
    }
};
