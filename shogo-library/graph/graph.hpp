#pragma once

/**
 * @file graph.hpp
 * @brief グラフ
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

/**
 * @brief グラフの汎用クラス
 *
 * @tparam Cost 辺のコストの型
 */
template <typename Cost = int>
struct Graph {
    /**
     * @brief 有向辺の構造体
     *
     * operator int()を定義しているので、int型にキャストすると勝手にdstになる
     * 例えば、
     * for (auto& e : g[v]) をすると、vから出る辺が列挙されるが、
     * for (int dst : g[v]) とすると、vから出る辺の行き先が列挙される
     */
    struct Edge {
        int src = -1;   //!< 始点
        int dst = -1;   //!< 終点
        Cost cost = {}; //!< コスト
        int id = -1;    //!< 辺の番号(追加された順、無向辺の場合はidが同じで方向が逆のものが2つ存在する)
        Edge() = default;
        Edge(int src, int dst, Cost cost = 1, int id = -1) : src(src), dst(dst), cost(cost), id(id) {}
        operator int() const { return dst; }
    };

    /**
     * @brief 最短路の結果
     *
     * auto [dist, prev] = g.shortest_path(s); のように受け取れる
     */
    struct ShortestPath {
        std::vector<Cost> dist;  //!< 各頂点への最短路長
        std::vector<Edge> prev;  //!< 各頂点への最短路上の直前の辺 (始点と未到達の頂点では src == -1)
        /**
         * @brief 始点から t への最短路を構成する辺のリスト (始点側から順に)
         * @note t が始点か未到達の頂点なら空。負閉路の影響を受ける頂点では最短路が定まらない (assert で落ちる)
         */
        std::vector<Edge> path(int t) const {
            std::vector<Edge> res;
            while (prev[t].src != -1) {
                res.push_back(prev[t]);
                assert(res.size() < prev.size() && "path: negative cycle");
                t = prev[t].src;
            }
            std::reverse(res.begin(), res.end());
            return res;
        }
    };

    int n;                            //!< 頂点数
    int m;                            //!< 辺数
    std::vector<std::vector<Edge>> g; //!< グラフの隣接リスト表現

    /**
     * @brief デフォルトコンストラクタ
     */
    Graph() : n(0), m(0), g(0) {}
    /**
     * @brief コンストラクタ
     * @param n 頂点数
     */
    explicit Graph(int n) : n(n), m(0), g(n) {}
    /**
     * @brief 無向辺を追加する
     * @param u 始点
     * @param v 終点
     * @param w コスト 省略したら1
     */
    void add_edge(int u, int v, Cost w = 1) {
        g[u].push_back({u, v, w, m});
        g[v].push_back({v, u, w, m++});
    }
    /**
     * @brief 有向辺を追加する
     * @param u 始点
     * @param v 終点
     * @param w コスト 省略したら1
     */
    void add_directed_edge(int u, int v, Cost w = 1) {
        g[u].push_back({u, v, w, m++});
    }
    /**
     * @brief 辺の情報を標準入力から受け取って追加する
     * @param m 辺の数
     * @param padding 頂点番号を入力からいくつずらすか 省略したら-1
     * @param weighted 辺の重みが入力されるか 省略したらfalseとなり、重み1で辺が追加される
     * @param directed 有向グラフかどうか 省略したらfalse
     */
    void read(int m, int padding = -1, bool weighted = false, bool directed = false) {
        for (int i = 0; i < m; i++) {
            int u, v;
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
    std::vector<Edge> &operator[](int v) {
        return g[v];
    }
    /**
     * @brief ある頂点から出る辺を列挙する
     * @param v 頂点番号
     * @return const std::vector<Edge>& vから出る辺のリスト
     */
    const std::vector<Edge> &operator[](int v) const {
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
        for (int i = 0; i < n; i++) {
            for (auto &e : g[i]) {
                res[e.id] = e;
            }
        }
        return res;
    }
    /**
     * @brief ある頂点から各頂点への最短路
     *
     * @param s 始点
     * @param weighted 1以外のコストの辺が存在するか 省略するとtrue
     * @param inf コストのminの単位元 未到達の頂点への距離はinfになる 省略すると
     * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
     */
    ShortestPath shortest_path(int s, bool weighted = true, Cost inf = -1) const {
        if (weighted)
            return shortest_path_dijkstra(s, inf);
        return shortest_path_bfs(s, inf);
    }
    /**
     * @brief ある頂点から各頂点への最短路
     *
     * @param s 始点
     * @param inf コストのminの単位元 未到達の頂点への距離はinfになる 省略すると std::numeric_limits<Cost>::max() / 4 負閉路の影響を受ける点は -inf
     * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
     */
    ShortestPath bellman_ford(int s, Cost inf = std::numeric_limits<Cost>::max() / 4) const {
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        dist[s] = Cost{};

        for (int i = 1; i < n; i++) {
            bool changed = false;
            for (int j = 0; j < n; j++)
                for (const auto &e : g[j]) {
                    if (dist[e.src] == inf)
                        continue;
                    if (dist[e.dst] > dist[e.src] + e.cost) {
                        dist[e.dst] = dist[e.src] + e.cost;
                        if (dist[e.dst] < -inf)
                            dist[e.dst] = -inf;
                        prev[e.dst] = e;
                        changed = true;
                    }
                }
            if (!changed)
                break;
        }
        for (int i = 0; i < n; i++) {
            bool changed = false;
            for (int j = 0; j < n; j++)
                for (const auto &e : g[j]) {
                    if (dist[e.src] == inf)
                        continue;
                    if (dist[e.dst] > dist[e.src] + e.cost) {
                        dist[e.dst] = -inf;
                        prev[e.dst] = e;
                        changed = true;
                    }
                }
            if (!changed)
                break;
        }
        return {dist, prev};
    }

  private:
    ShortestPath shortest_path_bfs(int s, Cost inf) const {
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        std::queue<int> que;
        dist[s] = 0;
        que.push(s);
        while (!que.empty()) {
            int u = que.front();
            que.pop();
            for (auto &e : g[u]) {
                if (dist[e.dst] == inf) {
                    dist[e.dst] = dist[e.src] + 1;
                    prev[e.dst] = e;
                    que.push(e.dst);
                }
            }
        }
        return {dist, prev};
    }
    ShortestPath shortest_path_dijkstra(int s, Cost inf) const {
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        using Node = std::pair<Cost, int>;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> que;
        dist[s] = 0;
        que.push({0, s});
        while (!que.empty()) {
            auto [d, u] = que.top();
            que.pop();
            if (d > dist[u])
                continue;
            for (auto &e : g[u]) {
                if (dist[e.dst] == inf || dist[e.dst] > dist[e.src] + e.cost) {
                    dist[e.dst] = dist[e.src] + e.cost;
                    prev[e.dst] = e;
                    que.push({dist[e.dst], e.dst});
                }
            }
        }
        return {dist, prev};
    }
};
