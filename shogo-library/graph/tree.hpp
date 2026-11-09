#pragma once

/**
 * @file tree.hpp
 * @brief 木の汎用テンプレート
 */

#include <algorithm>
#include <cassert>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/27998142f20ef4e094add3b53cc9c070db4ddbf8/src/graph/tree.hpp

template <typename>
class RootedTree;
template <typename>
class DoublingClimbTree;

/**
 * @brief 根なし木
 *
 * @tparam Cost = long long 辺の重み
 *
 * Graph<Cost>を継承し、すべて無向辺で表す
 */
template <typename Cost = long long>
struct Tree : private Graph<Cost> {
    // Graph<Cost>* g = new Tree<Cost>(); ができてしまうと、delete時にメモリリークが発生
    // 回避するためprivate継承にして、公開するメンバをusing宣言する
    // add_directed_edge は木でなくなるので公開しない (read は Tree::read で隠す)
    using Edge = typename Graph<Cost>::Edge;
    using Graph<Cost>::size;
    using Graph<Cost>::edge_count;
    using Graph<Cost>::Graph;
    using Graph<Cost>::add_edge;
    using Graph<Cost>::operator[];
    using Graph<Cost>::edges;

    /**
     * @brief コンストラクタ
     *
     * @param n ノード数
     */
    explicit Tree(long long n = 0) : Graph<Cost>(n) {}

    /**
     * @brief Graph として見る
     *
     * Graph を受け取る関数 (shortest_path_dijkstra など) に渡すときに使う
     * @return const Graph<Cost>& この木を表すグラフ
     */
    const Graph<Cost>& graph() const { return *this; }

    /**
     * @brief 辺の情報を標準入力から受け取って追加する
     * @param padding  = -1 頂点番号を入力からいくつずらすか
     * @param weighted = false 辺の重みが入力されるか
     */
    void read(long long padding = -1, bool weighted = false) {
        Graph<Cost>::read(this->size() - 1, padding, weighted, false);
    }

    /**
     * @brief 根付き木にする
     *
     * @param root 根
     * @return RootedTree<Cost> 根付き木のオブジェクト
     */
    [[nodiscard]]
    RootedTree<Cost> set_root(long long root) const& {
        return RootedTree<Cost>(*this, root);
    }
    /**
     * @brief 根付き木にして返す
     *
     * @param root 根
     * @return RootedTree<Cost> 根付き木のオブジェクト
     */
    [[nodiscard]]
    RootedTree<Cost> set_root(long long root) && {
        return RootedTree<Cost>(std::move(*this), root);
    }

    /**
     * @brief ダブリングLCAが使える根付き木を返す
     *
     * @param root 根
     * @return DoublingClimbTree<Cost> ダブリング済み根付き木のオブジェクト
     */
    [[nodiscard]]
    DoublingClimbTree<Cost> build_lca(long long root) const& {
        RootedTree rooted_tree(*this, root);
        return DoublingClimbTree<Cost>(std::move(rooted_tree));
    }
    /**
     * @brief ダブリングLCAが使える根付き木を返す
     *
     * @param root 根
     * @return DoublingClimbTree<Cost> ダブリング済み根付き木のオブジェクト
     */
    [[nodiscard]]
    DoublingClimbTree<Cost> build_lca(long long root) && {
        RootedTree rooted_tree(std::move(*this), root);
        return DoublingClimbTree<Cost>(std::move(rooted_tree));
    }
};

/**
 * @brief 根付き木
 *
 * @tparam Cost = long long 辺のコスト
 */
template <typename Cost = long long>
class RootedTree : private Tree<Cost> {
   public:
    using Edge = typename Tree<Cost>::Edge;
    using Tree<Cost>::size;
    using Tree<Cost>::edge_count;
    using Tree<Cost>::edges;
    using Tree<Cost>::graph;

   private:
    long long _root;                                           //!< 根
    std::vector<Edge> _par;                                    //!< 親へ向かう辺
    std::vector<std::vector<Edge>> _child;                     //!< 子へ向かう辺のリスト
    std::vector<Cost> _depth;                                  //!< 深さのリスト
    std::vector<Cost> _height;                                 //!< 部分木の高さのリスト
    std::vector<long long> _unweighted_depth;                  //!< 重みなしの深さのリスト
    std::vector<long long> _unweighted_height;                 //!< 重みなしの部分木の高さのリスト
    std::vector<long long> _sz;                                //!< 部分木の頂点数のリスト
    std::vector<long long> _preorder;                          //!< 先行順巡回
    std::vector<long long> _postorder;                         //!< 後行順巡回
    std::vector<std::pair<long long, long long>> _euler_tour;  //!< オイラーツアー DFS中にとおった頂点の(頂点番号,その頂点を通った回数)のリスト

   public:
    /**
     * @brief 頂点 v から出る辺を列挙する
     */
    const std::vector<Edge>& operator[](long long v) const { return Tree<Cost>::operator[](v); }

    /** @brief 根 */
    long long root() const { return _root; }
    /** @brief 頂点 v の親へ向かう辺 (根では dst == -1) */
    const Edge& parent_edge(long long v) const {
        assert(0 <= v && v < size());
        return _par[v];
    }
    /** @brief 頂点 v の子へ向かう辺 */
    const std::vector<Edge>& children(long long v) const {
        assert(0 <= v && v < size());
        return _child[v];
    }
    /** @brief 頂点 v の根からの距離 */
    Cost depth(long long v) const {
        assert(0 <= v && v < size());
        return _depth[v];
    }
    /** @brief 頂点 v の部分木の高さ */
    Cost height(long long v) const {
        assert(0 <= v && v < size());
        return _height[v];
    }
    /** @brief 頂点 v の根からの辺数 */
    long long unweighted_depth(long long v) const {
        assert(0 <= v && v < size());
        return _unweighted_depth[v];
    }
    /** @brief 頂点 v の部分木の高さ (辺数) */
    long long unweighted_height(long long v) const {
        assert(0 <= v && v < size());
        return _unweighted_height[v];
    }
    /** @brief 頂点 v の部分木の頂点数 */
    long long subtree_size(long long v) const {
        assert(0 <= v && v < size());
        return _sz[v];
    }
    /** @brief 行きがけ順 */
    const std::vector<long long>& preorder() const { return _preorder; }
    /** @brief 帰りがけ順 */
    const std::vector<long long>& postorder() const { return _postorder; }
    /** @brief オイラーツアー (頂点と訪問回数の組) */
    const std::vector<std::pair<long long, long long>>& euler_tour() const { return _euler_tour; }

    RootedTree() = delete;

    /**
     * @brief 親の頂点のリストから根が0の根付き木を構築するコンストラクタ
     *
     * @tparam T 頂点番号の整数型 (long long や int)
     * @param par_ 頂点0以外の親の頂点のリスト
     * @param padding = -1 parの頂点番号をいくつずらすか
     */
    template <typename T = long long>
    explicit RootedTree(const std::vector<T>& par_, long long padding = -1) : Tree<Cost>(par_.size() + 1), _root(0) {
        static_assert(std::is_integral_v<T>, "RootedTree: par_ must be a vector of integers");
        for (long long i = 0; i < static_cast<long long>(par_.size()); i++) {
            this->add_edge(i + 1, par_[i] + padding);
        }
        build();
    }
    /**
     * @brief Treeから根付き木を構築するコンストラクタ(コピー)
     *
     * @param tree Tree 頂点数1以上
     * @param root 根
     */
    RootedTree(const Tree<Cost>& tree, long long root) : Tree<Cost>(tree), _root(root) {
        build();
    }
    /**
     * @brief Treeから根付き木を構築するコンストラクタ(ムーブ)
     *
     * @param tree Tree
     * @param root 根
     */
    RootedTree(Tree<Cost>&& tree, long long root) : Tree<Cost>(std::move(tree)), _root(root) {
        build();
    }

    /**
     * @brief ダブリングLCAが使える根付き木を得る
     *
     * @return DoublingClimbTree<Cost> ダブリング済み根付き木のオブジェクト
     */
    [[nodiscard]]
    DoublingClimbTree<Cost> build_lca() const& {
        return DoublingClimbTree<Cost>(*this);
    }

    /**
     * @brief ダブリングLCAが使える根付き木を得る
     *
     * @return DoublingClimbTree<Cost> ダブリング済み根付き木のオブジェクト
     */
    [[nodiscard]]
    DoublingClimbTree<Cost> build_lca() && {
        return DoublingClimbTree<Cost>(std::move(*this));
    }

   private:
    void build() {
        long long n = this->size();
        assert(n > 0);
        assert(0 <= _root && _root < n);
        _par.resize(n);
        _par[_root] = {_root, -1, 0, -1};
        _child.resize(n);
        _depth.resize(n);
        _unweighted_depth.resize(n);
        _height.resize(n);
        _unweighted_height.resize(n);
        _sz.resize(n);
        std::vector<int> iter(n);
        std::stack<std::pair<int, int>> stk;
        stk.emplace(_root, 0);
        while (!stk.empty()) {
            auto [u, cnt] = stk.top();
            stk.pop();
            _euler_tour.emplace_back(u, cnt);
            if (iter[u] == 0) _preorder.push_back(u);
            while (iter[u] < static_cast<int>((*this)[u].size()) && (*this)[u][iter[u]].dst == _par[u]) iter[u]++;
            if (iter[u] == static_cast<int>((*this)[u].size())) {
                _postorder.push_back(u);
                _sz[u] = 1;
                _height[u] = 0;
                _unweighted_height[u] = 0;
                for (auto& e : _child[u]) {
                    _sz[u] += _sz[e.dst];
                    if (_height[u] < _height[e.dst] + e.cost) {
                        _height[u] = _height[e.dst] + e.cost;
                    }
                    if (_unweighted_height[u] < _unweighted_height[e.dst] + 1) {
                        _unweighted_height[u] = _unweighted_height[e.dst] + 1;
                    }
                }
            } else {
                auto& e = (*this)[u][iter[u]++];
                _par[e.dst] = {e.dst, u, e.cost, e.id};
                _child[u].push_back(e);
                _depth[e.dst] = _depth[u] + e.cost;
                _unweighted_depth[e.dst] = _unweighted_depth[u] + 1;
                stk.emplace(u, cnt + 1);
                stk.emplace(e.dst, 0);
            }
        }
    }
};

/**
 * @brief 親頂点ダブリング済み根付き木
 *
 * @tparam Cost = long long 辺の重み(注: hopでは根の重みは考えない)
 */
template <typename Cost = long long>
class DoublingClimbTree : private RootedTree<Cost> {
   public:
    using Edge = typename RootedTree<Cost>::Edge;
    using RootedTree<Cost>::size;
    using RootedTree<Cost>::edge_count;
    using RootedTree<Cost>::operator[];
    using RootedTree<Cost>::edges;
    using RootedTree<Cost>::graph;
    using RootedTree<Cost>::root;
    using RootedTree<Cost>::parent_edge;
    using RootedTree<Cost>::children;
    using RootedTree<Cost>::depth;
    using RootedTree<Cost>::height;
    using RootedTree<Cost>::unweighted_depth;
    using RootedTree<Cost>::unweighted_height;
    using RootedTree<Cost>::subtree_size;
    using RootedTree<Cost>::preorder;
    using RootedTree<Cost>::postorder;
    using RootedTree<Cost>::euler_tour;

   private:
    long long h;                                       //!< ダブリングの回数
    std::vector<std::vector<long long>> doubling_par;  //!< jから(1<<i)頂点登った頂点(存在しない場合は-1)

   public:
    DoublingClimbTree() = delete;

    /**
     * @brief RootedTreeからダブリング済み根付き木を構築するコンストラクタ(コピー)
     *
     * @param tree RootedTree
     */
    explicit DoublingClimbTree(const RootedTree<Cost>& tree) : RootedTree<Cost>(tree) {
        build();
    }

    /**
     * @brief RootedTreeからダブリング済み根付き木を構築するコンストラクタ(ムーブ)
     *
     * @param tree RootedTree
     */
    DoublingClimbTree(RootedTree<Cost>&& tree) : RootedTree<Cost>(std::move(tree)) {
        build();
    }

    /**
     * @brief 親の頂点のリストから根が0のダブリング済み根付き木を構築するコンストラクタ
     *
     * @tparam T 頂点番号の整数型 (long long や int)
     * @param par_ 頂点0以外の親の頂点のリスト
     * @param padding = -1 parの頂点番号をいくつずらすか
     */
    template <typename T = long long>
    explicit DoublingClimbTree(const std::vector<T>& par_, long long padding = -1) : RootedTree<Cost>(par_, padding) {
        build();
    }
    /**
     * @brief Treeからダブリング済み根付き木を構築するコンストラクタ(コピー)
     *
     * @param tree Tree
     * @param root 根
     */
    DoublingClimbTree(const Tree<Cost>& tree, long long root) : RootedTree<Cost>(tree, root) {
        build();
    }
    /**
     * @brief Treeからダブリング済み根付き木を構築するコンストラクタ(ムーブ)
     *
     * @param tree Tree
     * @param root 根
     */
    DoublingClimbTree(Tree<Cost>&& tree, long long root) : RootedTree<Cost>(std::move(tree), root) {
        build();
    }

    /**
     * @brief 頂点uからk回を根の方向に遡った頂点
     *
     * @param u 元の頂点
     * @param k 登る段数
     * @return long long k回登った頂点 (存在しなければ -1)
     */
    long long hop(long long u, long long k) const {
        assert(0 <= u && u < this->size() && k >= 0);
        if (k > this->unweighted_depth(u)) return -1;
        for (long long i = 0; i < h; i++) {
            if (k >> i & 1) u = doubling_par[i][u];
            if (u == -1) return -1;
        }
        return u;
    }

    /**
     * @brief 最小共通祖先
     *
     * @param u 頂点1
     * @param v 頂点2
     * @return long long LCAの頂点番号
     */
    long long lca(long long u, long long v) const {
        assert(0 <= u && u < this->size() && 0 <= v && v < this->size());
        if (this->unweighted_depth(u) > this->unweighted_depth(v)) std::swap(u, v);
        v = hop(v, this->unweighted_depth(v) - this->unweighted_depth(u));
        if (u == v) return u;
        for (long long i = h - 1; i >= 0; i--) {
            long long nu = doubling_par[i][u];
            long long nv = doubling_par[i][v];
            if (nu == -1) continue;
            if (nu != nv) {
                u = nu;
                v = nv;
            }
        }
        return this->parent_edge(u);
    }

    /**
     * @brief 頂点間距離(重み付き)
     *
     * @param u 頂点1
     * @param v 頂点2
     * @return Cost uとv間の最短経路の辺の重みの和
     */
    Cost dist(long long u, long long v) const {
        assert(0 <= u && u < this->size() && 0 <= v && v < this->size());
        return this->depth(u) + this->depth(v) - this->depth(lca(u, v)) * 2;
    }

   private:
    void build() {
        long long n = this->size();
        h = 1;
        while ((1LL << h) < n) h++;
        doubling_par.assign(h, std::vector<long long>(n, -1));
        for (long long i = 0; i < n; i++) doubling_par[0][i] = this->parent_edge(i);
        for (long long i = 0; i < h - 1; i++) {
            for (long long j = 0; j < n; j++) {
                if (doubling_par[i][j] != -1) {
                    doubling_par[i + 1][j] = doubling_par[i][doubling_par[i][j]];
                }
            }
        }
    }
};
