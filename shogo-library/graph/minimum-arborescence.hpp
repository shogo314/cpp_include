#pragma once

/**
 * @file minimum-arborescence.hpp
 * @brief 最小全域有向木
 */

#include <optional>
#include <utility>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/graph/minimum-arborescence.hpp

namespace internal_minimum_arborescence {

/**
 * @brief 全体への加算ができる leftist heap (ノードは添字で持つ)
 */
template <typename Cost>
class LeftistHeap {
   public:
    int make(Cost key, long long value) {
        nodes.push_back({key, Cost{}, value, -1, -1, 1});
        return static_cast<int>(nodes.size()) - 1;
    }
    int merge(int a, int b) {
        if (a == -1) return b;
        if (b == -1) return a;
        push(a), push(b);
        if (nodes[b].key < nodes[a].key) std::swap(a, b);
        nodes[a].r = merge(nodes[a].r, b);
        if (rank(nodes[a].l) < rank(nodes[a].r)) std::swap(nodes[a].l, nodes[a].r);
        nodes[a].rank = rank(nodes[a].r) + 1;
        return a;
    }
    std::pair<Cost, long long> top(int a) {
        push(a);
        return {nodes[a].key, nodes[a].value};
    }
    int pop(int a) {
        push(a);
        return merge(nodes[a].l, nodes[a].r);
    }
    void add(int a, Cost x) { nodes[a].lazy += x; }

   private:
    struct Node {
        Cost key, lazy;
        long long value;
        int l, r, rank;
    };
    std::vector<Node> nodes;
    int rank(int a) const { return a == -1 ? 0 : nodes[a].rank; }
    void push(int a) {
        if (nodes[a].lazy == Cost{}) return;
        nodes[a].key += nodes[a].lazy;
        if (nodes[a].l != -1) nodes[nodes[a].l].lazy += nodes[a].lazy;
        if (nodes[a].r != -1) nodes[nodes[a].r].lazy += nodes[a].lazy;
        nodes[a].lazy = Cost{};
    }
};

/**
 * @brief 巻き戻しのできる Union-Find (経路圧縮をしない)
 */
class RollbackUnionFind {
   public:
    explicit RollbackUnionFind(long long n) : parent_or_size(n, -1) {}
    long long find(long long x) const {
        while (parent_or_size[x] >= 0) x = parent_or_size[x];
        return x;
    }
    bool merge(long long x, long long y) {
        x = find(x), y = find(y);
        if (x == y) return false;
        if (parent_or_size[x] > parent_or_size[y]) std::swap(x, y);
        history.push_back({x, parent_or_size[x]});
        history.push_back({y, parent_or_size[y]});
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = x;
        return true;
    }
    long long time() const { return static_cast<long long>(history.size()); }
    void rollback(long long t) {
        while (time() > t) {
            parent_or_size[history.back().first] = history.back().second;
            history.pop_back();
        }
    }

   private:
    std::vector<long long> parent_or_size;
    std::vector<std::pair<long long, long long>> history;
};

}  // namespace internal_minimum_arborescence

/**
 * @brief 最小全域有向木の結果
 */
template <typename Cost>
struct MinimumArborescence {
    Cost cost;                                            //!< 使った辺のコストの総和
    std::vector<typename Graph<Cost>::Edge> parent_edge;  //!< 親から v に入る辺 (根では src == -1)
};

/**
 * @brief 有向グラフの最小全域有向木 (root を根とし、root から全頂点へ到達できる全域木で、辺のコストの総和が最小のもの)
 *
 * Chu-Liu/Edmonds 法を leftist heap と巻き戻しのできる Union-Find で O(m log m) にしたもの
 * @param root 根
 * @return std::optional<MinimumArborescence<Cost>> root から到達できない頂点があれば std::nullopt
 * @note 負のコストの辺があってもよい。自己ループ・多重辺があってもよい
 */
template <typename Cost = long long>
std::optional<MinimumArborescence<Cost>> minimum_arborescence(const Graph<Cost>& graph, long long root) {
    using Edge = typename Graph<Cost>::Edge;
    const long long n = graph.n;
    std::vector<Edge> es;
    for (long long v = 0; v < n; v++) {
        for (const auto& e : graph[v]) es.push_back(e);
    }
    internal_minimum_arborescence::LeftistHeap<Cost> heap;
    internal_minimum_arborescence::RollbackUnionFind uf(n);
    std::vector<int> top(n, -1);  // 各成分に入る辺のヒープ
    for (long long i = 0; i < static_cast<long long>(es.size()); i++) {
        top[es[i].dst] = heap.merge(top[es[i].dst], heap.make(es[i].cost, i));
    }
    struct Cycle {
        long long u, time;
        std::vector<long long> edges;
    };
    std::vector<Cycle> cycles;
    Cost cost{};
    std::vector<long long> seen(n, -1), path(n), queue(n), in(n, -1);
    seen[root] = root;
    for (long long s = 0; s < n; s++) {
        long long u = s, qi = 0;
        // s から入る辺を逆にたどり、閉路ができたら 1 つの頂点に縮める
        while (seen[u] < 0) {
            if (top[u] == -1) return std::nullopt;
            auto [w, i] = heap.top(top[u]);
            heap.add(top[u], -w);
            top[u] = heap.pop(top[u]);
            queue[qi] = i, path[qi++] = u, seen[u] = s;
            cost += w;
            u = uf.find(es[i].src);
            if (seen[u] == s) {
                int cycle = -1;
                long long end = qi, time = uf.time(), v;
                do {
                    v = path[--qi];
                    cycle = heap.merge(cycle, top[v]);
                } while (uf.merge(u, v));
                u = uf.find(u), top[u] = cycle, seen[u] = -1;
                cycles.push_back({u, time, std::vector<long long>(queue.begin() + qi, queue.begin() + end)});
            }
        }
        for (long long i = 0; i < qi; i++) in[uf.find(es[queue[i]].dst)] = queue[i];
    }
    // 縮めた閉路を新しいものから戻し、閉路の中の辺を選び直す
    for (auto it = cycles.rbegin(); it != cycles.rend(); ++it) {
        uf.rollback(it->time);
        long long in_edge = in[it->u];
        for (long long i : it->edges) in[uf.find(es[i].dst)] = i;
        in[uf.find(es[in_edge].dst)] = in_edge;
    }
    std::vector<Edge> parent_edge(n);
    for (long long v = 0; v < n; v++) {
        if (v != root) parent_edge[v] = es[in[v]];
    }
    return MinimumArborescence<Cost>{cost, parent_edge};
}
