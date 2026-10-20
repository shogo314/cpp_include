#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/data_structures/persistent-array.hpp

/**
 * @file persistent-array.hpp
 * @brief 永続配列
 */

#include <array>
#include <cassert>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

/**
 * @brief 永続配列
 *
 * 更新すると新しい版を返し、元の版は変わらない。同じ配列から作った版はノードを共有する
 * 例:
 * PersistentArray<int> a0(n, -1);
 * auto a1 = a0.set(3, 10);  // a0 はそのまま
 * a1[3];                    // 10
 *
 * @tparam T 要素の型
 */
template <typename T>
class PersistentArray {
    // すべての版のノードを持つ。ノードは削除しない
    struct Pool {
        std::vector<std::array<int, 2>> child;  // 内部ノードの子 (最下段の内部ノードでは葉の番号)
        std::vector<T> leaf;
        int make_leaf(const T& x) {
            leaf.push_back(x);
            return static_cast<int>(leaf.size()) - 1;
        }
        int make_node(int l, int r) {
            child.push_back({l, r});
            return static_cast<int>(child.size()) - 1;
        }
    };

    std::shared_ptr<Pool> pool;  // 要素数 0 なら nullptr
    long long n;
    int height, root;  // height = 0 なら root は葉の番号

    PersistentArray(std::shared_ptr<Pool> pool, long long n, int height, int root) : pool(std::move(pool)), n(n), height(height), root(root) {}

    void init_height() {
        height = 0;
        while ((1LL << height) < n) height++;
    }
    // t ([a, a + 2^h)) の [l, r) を u に置き換える
    int copy_rec(int t, int u, long long a, int h, long long l, long long r) const {
        if (r <= a || a + (1LL << h) <= l) return t;
        if (l <= a && a + (1LL << h) <= r) return u;
        int lc = copy_rec(pool->child[t][0], pool->child[u][0], a, h - 1, l, r);
        int rc = copy_rec(pool->child[t][1], pool->child[u][1], a + (1LL << (h - 1)), h - 1, l, r);
        return pool->make_node(lc, rc);
    }
    int build(const std::vector<T>& v, long long a, int h) {
        if (h == 0) return pool->make_leaf(a < n ? v[a] : T());
        int l = build(v, a, h - 1);
        int r = build(v, a + (1LL << (h - 1)), h - 1);
        return pool->make_node(l, r);
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * 同じ値の部分木は共有するので、n が大きくても O(log n) で作れる
     *
     * @param n 要素数
     * @param x 初期値
     */
    explicit PersistentArray(long long n = 0, const T& x = T()) : pool(nullptr), n(n), height(0), root(-1) {
        assert(0 <= n);
        if (n == 0) return;
        pool = std::make_shared<Pool>();
        init_height();
        root = pool->make_leaf(x);
        for (int h = 0; h < height; h++) root = pool->make_node(root, root);
    }
    /**
     * @brief コンストラクタ
     *
     * @param v 初期の要素
     */
    explicit PersistentArray(const std::vector<T>& v) : pool(nullptr), n(v.size()), height(0), root(-1) {
        if (n == 0) return;
        pool = std::make_shared<Pool>();
        init_height();
        root = build(v, 0, height);
    }

    /**
     * @brief 要素数を返す
     */
    long long size() const { return n; }

    /**
     * @brief 指定された要素の値を返す
     *
     * @param k インデックス
     * @return T 値
     */
    T get(long long k) const {
        assert(0 <= k && k < n);
        int t = root;
        for (int h = height - 1; h >= 0; h--) t = pool->child[t][k >> h & 1];
        return pool->leaf[t];
    }
    /**
     * @brief 指定された要素の値を返す
     *
     * @param k インデックス
     * @return T 値
     */
    T operator[](long long k) const { return get(k); }

    /**
     * @brief 指定された要素の値をxにした新しい版を返す
     *
     * @param k インデックス
     * @param x 新しい値
     * @return PersistentArray 新しい版 (自身は変わらない)
     */
    PersistentArray set(long long k, const T& x) const {
        assert(0 <= k && k < n);
        std::vector<int> path(height);
        int t = root;
        for (int h = height - 1; h >= 0; h--) {
            path[h] = t;
            t = pool->child[t][k >> h & 1];
        }
        t = pool->make_leaf(x);
        for (int h = 0; h < height; h++) {
            std::array<int, 2> c = pool->child[path[h]];
            c[k >> h & 1] = t;
            t = pool->make_node(c[0], c[1]);
        }
        return PersistentArray(pool, n, height, t);
    }
    /**
     * @brief [l, r)の区間を別の版の値にした新しい版を返す
     *
     * @param other 写す元の版 (自身と同じ配列から作ったもの)
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @return PersistentArray 新しい版 (自身も other も変わらない)
     */
    PersistentArray copy(const PersistentArray& other, long long l, long long r) const {
        assert(pool == other.pool);
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return *this;
        return PersistentArray(pool, n, height, copy_rec(root, other.root, 0, height, l, r));
    }

    /**
     * @brief 内容を出力する
     *
     * @tparam CharT 出力ストリームの文字型
     * @tparam Traits 出力ストリームの文字型特性
     * @param os 出力ストリーム
     * @param rhs 永続配列
     * @return std::basic_ostream<CharT, Traits>& 出力ストリーム
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const PersistentArray& rhs) {
        for (long long i = 0; i < rhs.n; i++) {
            if (i != 0) os << CharT(' ');
            os << rhs[i];
        }
        return os;
    }

    /**
     * @brief すべての要素を並べた列を返す
     */
    std::vector<T> to_vector() const {
        std::vector<T> res(n);
        for (long long i = 0; i < n; i++) res[i] = get(i);
        return res;
    }
};
