#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/persistent-queue.hpp

/**
 * @file persistent-queue.hpp
 * @brief 永続キュー
 */

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

/**
 * @brief 永続キュー
 *
 * 更新すると新しい版を返し、元の版は変わらない。同じキューから作った版はノードを共有する
 * 要素を木の根から末尾へのパスとして持ち、先頭はジャンプポインタで祖先をたどって求める
 * 例:
 * PersistentQueue<int> q0;
 * auto q1 = q0.push(1).push(2);  // q0 はそのまま
 * q1.front();                    // 1
 * q1.pop().front();              // 2
 *
 * @tparam T 要素の型
 */
template <typename T>
class PersistentQueue {
    // すべての版のノードを持つ。ノードは削除しない。ノード 0 は根 (要素を持たない)
    struct Pool {
        std::vector<T> val;
        std::vector<int> parent, jump, depth;
        Pool() : val(1), parent{0}, jump{0}, depth{0} {}
        int make_node(const T& x, int p) {
            // ジャンプの長さが 1, 1, 3, 1, 1, 3, 7, ... (skew binary) になるように張る
            int j = jump[p];
            int nj = depth[p] - depth[j] == depth[j] - depth[jump[j]] ? jump[j] : p;
            val.push_back(x);
            parent.push_back(p);
            jump.push_back(nj);
            depth.push_back(depth[p] + 1);
            return static_cast<int>(val.size()) - 1;
        }
        // t の祖先で深さが d のもの
        int ancestor(int t, int d) const {
            while (depth[t] > d) t = depth[jump[t]] >= d ? jump[t] : parent[t];
            return t;
        }
    };

    std::shared_ptr<Pool> pool;  // 一度も push していなければ nullptr
    int back_node, n;            // 要素は back_node から n 個上までのノード

    PersistentQueue(std::shared_ptr<Pool> pool, int back_node, int n) : pool(std::move(pool)), back_node(back_node), n(n) {}

   public:
    /**
     * @brief 空のキューを作る
     */
    PersistentQueue() : pool(nullptr), back_node(0), n(0) {}

    /**
     * @brief 要素数を返す
     */
    long long size() const { return n; }
    /**
     * @brief 空かどうかを返す
     */
    bool empty() const { return n == 0; }

    /**
     * @brief 先頭の要素を返す
     */
    T front() const {
        assert(!empty());
        return pool->val[pool->ancestor(back_node, pool->depth[back_node] - n + 1)];
    }
    /**
     * @brief 末尾の要素を返す
     */
    T back() const {
        assert(!empty());
        return pool->val[back_node];
    }
    /**
     * @brief 末尾にxを加えた新しい版を返す
     *
     * @param x 加える値
     * @return PersistentQueue 新しい版 (自身は変わらない)
     */
    PersistentQueue push(const T& x) const {
        std::shared_ptr<Pool> p = pool ? pool : std::make_shared<Pool>();
        int t = p->make_node(x, back_node);
        return PersistentQueue(std::move(p), t, n + 1);
    }
    /**
     * @brief 先頭の要素を取り除いた新しい版を返す
     *
     * @return PersistentQueue 新しい版 (自身は変わらない)
     */
    PersistentQueue pop() const {
        assert(!empty());
        return PersistentQueue(pool, back_node, n - 1);
    }

    /**
     * @brief すべての要素を先頭から順に並べた列を返す
     */
    std::vector<T> to_vector() const {
        std::vector<T> res(n);
        int t = back_node;
        for (int i = n - 1; i >= 0; i--) {
            res[i] = pool->val[t];
            t = pool->parent[t];
        }
        return res;
    }
};
