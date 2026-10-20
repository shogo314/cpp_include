#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/graph/functional-graph.hpp

/**
 * @file functional-graph.hpp
 * @brief ファンクショナルグラフ
 */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <type_traits>
#include <vector>

/**
 * @brief N頂点N辺の有向グラフで、すべての頂点の出次数が1
 */
struct FunctionalGraph {
    long long n;                                   //!< 頂点数
    std::vector<long long> g;                      //!< グラフ
    std::vector<bool> is_cycle;                    //!< サイクル上の頂点か
    std::vector<long long> depth;                  //!< サイクル上の頂点までの距離
    std::vector<long long> root;                   //!< 最も近いサイクル上の頂点
    std::vector<std::vector<long long>> groups;    //!< 各連結成分のリスト
    std::vector<std::vector<long long>> cycles;    //!< 各連結成分のサイクル
    std::vector<long long> group_id;               //!< その頂点が属する連結成分のindex
    std::vector<long long> root_index;             //!< rootのcycles上のindex
    std::vector<std::vector<long long>> doubling;  //!< DoublingClimbTree

    void build() {
        assert(static_cast<long long>(g.size()) == n);
        for (long long i = 0; i < n; i++) assert(0 <= g[i] && g[i] < n);
        is_cycle.assign(n, false);
        depth.assign(n, 0);
        root.assign(n, -1);
        group_id.assign(n, 0);
        root_index.assign(n, 0);
        groups.clear();
        cycles.clear();
        std::vector<bool> visited(n);
        for (long long i = 0; i < n; i++) {
            if (visited[i]) continue;
            std::vector<long long> v;
            long long u = i;
            while (!visited[u]) {
                visited[u] = true;
                v.push_back(u);
                u = g[u];
            }
            if (root[u] == -1) {
                long long k = std::find(v.begin(), v.end(), u) - v.begin();
                cycles.push_back({});
                groups.push_back({});
                for (long long j = 0; j < k; j++) {
                    is_cycle[v[j]] = false;
                    root[v[j]] = u;
                    root_index[v[j]] = 0;
                    group_id[v[j]] = cycles.size() - 1;
                    depth[v[j]] = k - j;
                    groups.back().push_back(v[j]);
                }
                for (long long j = k; j < static_cast<long long>(v.size()); j++) {
                    is_cycle[v[j]] = true;
                    root[v[j]] = v[j];
                    cycles.back().push_back(v[j]);
                    root_index[v[j]] = cycles.back().size() - 1;
                    group_id[v[j]] = cycles.size() - 1;
                    depth[v[j]] = 0;
                    groups.back().push_back(v[j]);
                }
            } else {
                for (long long j = 0; j < static_cast<long long>(v.size()); j++) {
                    is_cycle[v[j]] = false;
                    root[v[j]] = root[u];
                    root_index[v[j]] = root_index[u];
                    group_id[v[j]] = group_id[u];
                    depth[v[j]] = depth[u] + static_cast<long long>(v.size()) - j;
                    groups[group_id[u]].push_back(v[j]);
                }
            }
        }
        doubling.assign(n, {});
        std::vector<long long> tmp;
        for (long long i = 0; i < n; i++) {
            if (is_cycle[i]) continue;
            tmp.push_back(i);
            doubling[i].push_back(g[i]);
        }
        for (int k = 0; !tmp.empty(); k++) {
            std::vector<long long> next_tmp;
            for (long long i : tmp) {
                if (k < static_cast<int>(doubling[doubling[i][k]].size())) {
                    doubling[i].push_back(doubling[doubling[i][k]][k]);
                    next_tmp.push_back(i);
                }
            }
            tmp.swap(next_tmp);
        }
    }

    /**
     * @brief デフォルトコンストラクタ
     */
    FunctionalGraph() : n(0), g(0) {}
    /**
     * @brief コンストラクタ
     * @param n 頂点数
     */
    explicit FunctionalGraph(long long n) : n(n), g(n) {}
    /**
     * @brief コンストラクタ
     * @tparam T 頂点番号の整数型 (long long や int)
     * @param g i -> g[i]
     */
    template <typename T = long long>
    explicit FunctionalGraph(const std::vector<T>& g) : n(g.size()), g(g.begin(), g.end()) {
        static_assert(std::is_integral_v<T>, "FunctionalGraph: g must be a vector of integers");
        build();
    }
    /**
     * @brief 頂点数
     */
    long long size() const { return n; }
    /**
     * @brief 辺の情報を標準入力から受け取って追加する
     * @param padding 頂点番号を入力からいくつずらすか 省略したら-1
     */
    void read(long long padding = -1) {
        for (long long i = 0; i < n; i++) {
            std::cin >> g[i];
            g[i] += padding;
        }
        build();
    }
    /**
     * @brief 頂点xからk回移動した頂点
     *
     * @param x 0 <= x < n
     * @param k 0 <= k
     * @return long long 頂点番号
     */
    long long hop(long long x, long long k) const {
        assert(0 <= x && x < n && k >= 0);
        if (k >= depth[x]) {
            k -= depth[x];
            x = root[x];
            k %= cycles[group_id[x]].size();
            long long y = root_index[x] + k;
            if (static_cast<long long>(cycles[group_id[x]].size()) <= y) {
                y -= cycles[group_id[x]].size();
            }
            return cycles[group_id[x]][y];
        }
        for (int i = 0; k; i++) {
            if (k & 1) {
                x = doubling[x][i];
            }
            k >>= 1;
        }
        return x;
    }
    /**
     * @brief 頂点xから0回以上の移動でたどり着ける頂点の個数
     */
    long long hopable(long long x) const {
        assert(0 <= x && x < n);
        return depth[x] + static_cast<long long>(cycles[group_id[x]].size());
    }
};
