#pragma once
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/data_structures/erasable-priority-queue.hpp

/**
 * @file erasable-priority-queue.hpp
 * @brief 削除可能な優先度付きキュー
 */

#include <cassert>
#include <functional>
#include <queue>
#include <vector>

/**
 * @brief 削除可能な優先度付きキュー
 *
 * std::priority_queue と同じく、既定では最大の要素を取り出す
 * 削除する要素を別のヒープに入れておき、先頭に来たときに取り除く。各操作は償却 O(log n)
 * 例:
 * ErasablePriorityQueue<int> pq;
 * pq.push(3); pq.push(5); pq.erase(5); pq.top();  // 3
 *
 * @tparam T 要素の型
 * @tparam Compare 比較関数。std::greater<T> にすると最小の要素を取り出す
 */
template <typename T, typename Compare = std::less<T>>
class ErasablePriorityQueue {
    std::priority_queue<T, std::vector<T>, Compare> que, del;
    Compare comp;

    bool equivalent(const T& x, const T& y) const { return !comp(x, y) && !comp(y, x); }
    // 先頭が削除予定なら取り除く
    void normalize() {
        while (!del.empty() && equivalent(que.top(), del.top())) {
            que.pop();
            del.pop();
        }
    }

   public:
    explicit ErasablePriorityQueue(Compare comp = Compare()) : que(comp), del(comp), comp(comp) {}

    /**
     * @brief 要素数
     */
    long long size() const { return que.size() - del.size(); }
    bool empty() const { return size() == 0; }

    /**
     * @brief x を追加する
     */
    void push(const T& x) {
        que.push(x);
        normalize();
    }
    /**
     * @brief x を 1 つ削除する。x は含まれていること (含まれていない要素を削除すると壊れる)
     * 比較関数で等しい要素は区別しない
     */
    void erase(const T& x) {
        del.push(x);
        normalize();
    }
    /**
     * @brief 先頭 (既定では最大) の要素
     */
    const T& top() const {
        assert(!empty());
        return que.top();
    }
    /**
     * @brief 先頭の要素を削除する
     */
    void pop() {
        assert(!empty());
        que.pop();
        normalize();
    }
};
