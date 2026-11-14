#pragma once
// https://github.com/shogo314/shogo-library/blob/adb06bae4b0e202b3a1fe53ef9f9a1b30077d8cc/src/graph/functional-graph.hpp

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
class FunctionalGraph {
    long long n;  //!< 頂点数

    std::vector<long long> g;

    std::vector<bool> _is_cycle;                   //!< サイクル上の頂点か
    std::vector<long long> _depth;                 //!< サイクル上の頂点までの距離
    std::vector<long long> _root;                  //!< 最も近いサイクル上の頂点
    std::vector<std::vector<long long>> _groups;   //!< 各連結成分のリスト
    std::vector<std::vector<long long>> _cycles;   //!< 各連結成分のサイクル
    std::vector<long long> _group_id;              //!< その頂点が属する連結成分のindex
    std::vector<long long> _root_index;            //!< rootのcycles上のindex
    std::vector<std::vector<long long>> doubling;  //!< DoublingClimbTree

    bool built = false;

    void build() {
        assert(static_cast<long long>(g.size()) == n);
        for (long long i = 0; i < n; i++) assert(0 <= g[i] && g[i] < n);
        _is_cycle.assign(n, false);
        _depth.assign(n, 0);
        _root.assign(n, -1);
        _group_id.assign(n, 0);
        _root_index.assign(n, 0);
        _groups.clear();
        _cycles.clear();
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
            if (_root[u] == -1) {
                long long k = std::find(v.begin(), v.end(), u) - v.begin();
                _cycles.push_back({});
                _groups.push_back({});
                for (long long j = 0; j < k; j++) {
                    _is_cycle[v[j]] = false;
                    _root[v[j]] = u;
                    _root_index[v[j]] = 0;
                    _group_id[v[j]] = _cycles.size() - 1;
                    _depth[v[j]] = k - j;
                    _groups.back().push_back(v[j]);
                }
                for (long long j = k; j < static_cast<long long>(v.size()); j++) {
                    _is_cycle[v[j]] = true;
                    _root[v[j]] = v[j];
                    _cycles.back().push_back(v[j]);
                    _root_index[v[j]] = _cycles.back().size() - 1;
                    _group_id[v[j]] = _cycles.size() - 1;
                    _depth[v[j]] = 0;
                    _groups.back().push_back(v[j]);
                }
            } else {
                for (long long j = 0; j < static_cast<long long>(v.size()); j++) {
                    _is_cycle[v[j]] = false;
                    _root[v[j]] = _root[u];
                    _root_index[v[j]] = _root_index[u];
                    _group_id[v[j]] = _group_id[u];
                    _depth[v[j]] = _depth[u] + static_cast<long long>(v.size()) - j;
                    _groups[_group_id[u]].push_back(v[j]);
                }
            }
        }
        doubling.assign(n, {});
        std::vector<long long> tmp;
        for (long long i = 0; i < n; i++) {
            if (_is_cycle[i]) continue;
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
        built = true;
    }

   public:
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
    /** @brief 頂点 v がサイクル上にあるか */
    bool is_cycle(long long v) const {
        assert(built && 0 <= v && v < n);
        return _is_cycle[v];
    }
    /** @brief 頂点 v のサイクルまでの距離 */
    long long depth(long long v) const {
        assert(built && 0 <= v && v < n);
        return _depth[v];
    }
    /** @brief 頂点 v の最初に着くサイクル上の頂点 */
    long long root(long long v) const {
        assert(built && 0 <= v && v < n);
        return _root[v];
    }
    /** @brief 頂点 v の連結成分の番号 */
    long long group_id(long long v) const {
        assert(built && 0 <= v && v < n);
        return _group_id[v];
    }
    /** @brief 頂点 v の最初に着くサイクル上の頂点の、サイクル内での位置 */
    long long root_index(long long v) const {
        assert(built && 0 <= v && v < n);
        return _root_index[v];
    }
    /** @brief 連結成分の一覧 */
    const std::vector<std::vector<long long>>& groups() const {
        assert(built);
        return _groups;
    }
    /** @brief 各連結成分のサイクル (辺をたどる順) */
    const std::vector<std::vector<long long>>& cycles() const {
        assert(built);
        return _cycles;
    }
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
        assert(built && 0 <= x && x < n && k >= 0);
        if (k >= _depth[x]) {
            k -= _depth[x];
            x = _root[x];
            k %= _cycles[_group_id[x]].size();
            long long y = _root_index[x] + k;
            if (static_cast<long long>(_cycles[_group_id[x]].size()) <= y) {
                y -= _cycles[_group_id[x]].size();
            }
            return _cycles[_group_id[x]][y];
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
        assert(built && 0 <= x && x < n);
        return _depth[x] + static_cast<long long>(_cycles[_group_id[x]].size());
    }
};
