#pragma once
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/graph/graph.hpp

/**
 * @file graph.hpp
 * @brief グラフ
 */

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief グラフの汎用クラス
 *
 * @tparam Cost 辺のコストの型
 */
template <typename Cost = int>
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
        std::vector<Edge> path(long long t) const {
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
    /**
     * @brief ある頂点から各頂点への最短路
     *
     * @param s 始点
     * @param weighted 1以外のコストの辺が存在するか 省略するとtrue
     * @param inf コストのminの単位元 未到達の頂点への距離はinfになる 省略すると std::numeric_limits<Cost>::max() / 4
     * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
     */
    ShortestPath shortest_path(long long s, bool weighted = true, Cost inf = std::numeric_limits<Cost>::max() / 4) const {
        assert(0 <= s && s < n);
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
    ShortestPath bellman_ford(long long s, Cost inf = std::numeric_limits<Cost>::max() / 4) const {
        assert(0 <= s && s < n);
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        dist[s] = Cost{};

        for (long long i = 1; i < n; i++) {
            bool changed = false;
            for (long long j = 0; j < n; j++)
                for (const auto& e : g[j]) {
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
        for (long long i = 0; i < n; i++) {
            bool changed = false;
            for (long long j = 0; j < n; j++)
                for (const auto& e : g[j]) {
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
    /**
     * @brief 辺のコストが 0 か 1 だけのグラフで、ある頂点から各頂点への最短路 (01-BFS) O(n + m)
     *
     * @param s 始点
     * @param inf 未到達の頂点への距離 省略すると std::numeric_limits<Cost>::max() / 4
     * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
     */
    ShortestPath shortest_path_01bfs(long long s, Cost inf = std::numeric_limits<Cost>::max() / 4) const {
        assert(0 <= s && s < n);
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        std::deque<long long> que;
        dist[s] = 0;
        que.push_back(s);
        while (!que.empty()) {
            long long u = que.front();
            que.pop_front();
            for (auto& e : g[u]) {
                assert(e.cost == 0 || e.cost == 1);
                if (dist[e.dst] == inf || dist[e.dst] > dist[u] + e.cost) {
                    dist[e.dst] = dist[u] + e.cost;
                    prev[e.dst] = e;
                    if (e.cost == 0)
                        que.push_front(e.dst);
                    else
                        que.push_back(e.dst);
                }
            }
        }
        return {dist, prev};
    }
    /**
     * @brief 全点対間の最短路長 (Floyd-Warshall) O(n^3)
     *
     * @param inf 未到達を表す値 省略すると std::numeric_limits<Cost>::max() / 4
     * @return std::vector<std::vector<Cost>> dist[i][j] が i から j への最短路長 (到達できなければ inf)
     * @note 負の辺があってもよい。dist[i][i] < 0 となる i があれば負閉路がある
     */
    std::vector<std::vector<Cost>> floyd_warshall(Cost inf = std::numeric_limits<Cost>::max() / 4) const {
        std::vector<std::vector<Cost>> dist(n, std::vector<Cost>(n, inf));
        for (long long i = 0; i < n; i++) dist[i][i] = Cost{};
        for (long long i = 0; i < n; i++) {
            for (auto& e : g[i]) dist[i][e.dst] = std::min(dist[i][e.dst], e.cost);
        }
        for (long long k = 0; k < n; k++) {
            for (long long i = 0; i < n; i++) {
                if (dist[i][k] == inf) continue;
                for (long long j = 0; j < n; j++) {
                    if (dist[k][j] == inf) continue;
                    dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
                }
            }
        }
        return dist;
    }

   private:
    ShortestPath shortest_path_bfs(long long s, Cost inf) const {
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        std::queue<long long> que;
        dist[s] = 0;
        que.push(s);
        while (!que.empty()) {
            long long u = que.front();
            que.pop();
            for (auto& e : g[u]) {
                if (dist[e.dst] == inf) {
                    dist[e.dst] = dist[e.src] + 1;
                    prev[e.dst] = e;
                    que.push(e.dst);
                }
            }
        }
        return {dist, prev};
    }
    ShortestPath shortest_path_dijkstra(long long s, Cost inf) const {
        std::vector<Cost> dist(n, inf);
        std::vector<Edge> prev(n);
        using Node = std::pair<Cost, long long>;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> que;
        dist[s] = 0;
        que.push({0, s});
        while (!que.empty()) {
            auto [d, u] = que.top();
            que.pop();
            if (d > dist[u])
                continue;
            for (auto& e : g[u]) {
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
