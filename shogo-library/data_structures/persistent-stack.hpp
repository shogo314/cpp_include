#pragma once
// https://github.com/shogo314/shogo-library/blob/b591e1ab6db09d0025e385b1bb5f4d8982cc4d1f/src/data_structures/persistent-stack.hpp

/**
 * @file persistent-stack.hpp
 * @brief 永続スタック
 */

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

/**
 * @brief 永続スタック
 *
 * 更新すると新しい版を返し、元の版は変わらない。同じスタックから作った版はノードを共有する
 * 例:
 * PersistentStack<int> s0;
 * auto s1 = s0.push(1).push(2);  // s0 はそのまま
 * s1.top();                      // 2
 * s1.pop().top();                // 1
 *
 * @tparam T 要素の型
 */
template <typename T>
class PersistentStack {
    // すべての版のノードを持つ。ノードは削除しない
    struct Pool {
        std::vector<T> val;
        std::vector<int> parent;  // 1 つ下の要素のノード (一番下なら -1)
        int make_node(const T& x, int p) {
            val.push_back(x);
            parent.push_back(p);
            return int(val.size()) - 1;
        }
    };

    std::shared_ptr<Pool> pool;  // 一度も push していなければ nullptr
    int top_node, n;             // 空なら top_node = -1

    PersistentStack(std::shared_ptr<Pool> pool, int top_node, int n) : pool(std::move(pool)), top_node(top_node), n(n) {}

   public:
    /**
     * @brief 空のスタックを作る
     */
    PersistentStack() : pool(nullptr), top_node(-1), n(0) {}

    /**
     * @brief 要素数を返す
     */
    long long size() const { return n; }
    /**
     * @brief 空かどうかを返す
     */
    bool empty() const { return n == 0; }

    /**
     * @brief 一番上の要素を返す
     */
    T top() const {
        assert(!empty());
        return pool->val[top_node];
    }
    /**
     * @brief xを積んだ新しい版を返す
     *
     * @param x 積む値
     * @return PersistentStack 新しい版 (自身は変わらない)
     */
    PersistentStack push(const T& x) const {
        std::shared_ptr<Pool> p = pool ? pool : std::make_shared<Pool>();
        int t = p->make_node(x, top_node);
        return PersistentStack(std::move(p), t, n + 1);
    }
    /**
     * @brief 一番上の要素を取り除いた新しい版を返す
     *
     * @return PersistentStack 新しい版 (自身は変わらない)
     */
    PersistentStack pop() const {
        assert(!empty());
        return PersistentStack(pool, pool->parent[top_node], n - 1);
    }

    /**
     * @brief すべての要素を下から順に並べた列を返す
     */
    std::vector<T> to_vector() const {
        std::vector<T> res(n);
        int t = top_node;
        for (int i = n - 1; i >= 0; i--) {
            res[i] = pool->val[t];
            t = pool->parent[t];
        }
        return res;
    }
};
