#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/string/trie.hpp

/**
 * @file trie.hpp
 * @brief Trie
 */

#include <array>
#include <cassert>
#include <vector>

/**
 * @brief Trie (文字列の集合を接頭辞の木で管理する)
 *
 * 文字 c は c - Base として 0 以上 K 未満の番号に直す。根はノード 0 (空文字列)
 * 同じ文字列を何度も追加でき、多重集合として扱う
 * 例:
 * Trie<26, 'a'> trie;
 * trie.insert("abc");
 * trie.count_prefix("ab");  // 1
 *
 * @tparam K 文字の種類数
 * @tparam Base 最小の文字
 */
template <int K = 26, int Base = 'a'>
class Trie {
    struct Node {
        std::array<int, K> next;
        int parent, depth;
        int prefix_cnt;  // このノードを通る (このノードの文字列を接頭辞に持つ) 文字列の個数
        int word_cnt;    // このノードで終わる文字列の個数
    };
    // メモリを抑えるため、ノードの中身は int で持つ。外へ返すときに long long にする
    std::vector<Node> nodes;

    int add_node(int parent, int depth) {
        Node node;
        node.next.fill(-1);
        node.parent = parent;
        node.depth = depth;
        node.prefix_cnt = node.word_cnt = 0;
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    static int index(int c) {
        assert(0 <= c - Base && c - Base < K);
        return c - Base;
    }
    // 文字列 s に対応するノード (なければ -1)
    template <typename R>
    int find_node(const R& s) const {
        int v = 0;
        for (auto c : s) {
            v = nodes[v].next[index(c)];
            if (v == -1) return -1;
        }
        return v;
    }

   public:
    Trie() { add_node(-1, 0); }

    /**
     * @brief ノードの個数 (根を含む)
     */
    long long size() const { return nodes.size(); }

    /**
     * @brief 文字列 s を 1 個追加し、s に対応するノードを返す O(|s|)
     * @param s std::string や std::vector<int> など、要素を整数に変換できる列
     */
    template <typename R>
    long long insert(const R& s) {
        int v = 0;
        nodes[v].prefix_cnt++;
        for (auto c : s) {
            int i = index(c);
            if (nodes[v].next[i] == -1) {
                int u = add_node(v, nodes[v].depth + 1);
                nodes[v].next[i] = u;
            }
            v = nodes[v].next[i];
            nodes[v].prefix_cnt++;
        }
        nodes[v].word_cnt++;
        return v;
    }

    /**
     * @brief 文字列 s を 1 個取り除く。s がなければ何もせず false を返す O(|s|)
     * ノードは削除しないので、ノードの番号は変わらない
     */
    template <typename R>
    bool erase(const R& s) {
        int v = find_node(s);
        if (v == -1 || nodes[v].word_cnt == 0) return false;
        nodes[v].word_cnt--;
        for (; v != -1; v = nodes[v].parent) nodes[v].prefix_cnt--;
        return true;
    }

    /**
     * @brief 文字列 s に対応するノード (なければ -1) O(|s|)
     * s を接頭辞に持つ文字列を追加したことがあれば、取り除いた後でもノードは残る
     */
    template <typename R>
    long long find(const R& s) const { return find_node(s); }

    /**
     * @brief 文字列 s の個数 O(|s|)
     */
    template <typename R>
    long long count(const R& s) const {
        int v = find_node(s);
        return v == -1 ? 0 : nodes[v].word_cnt;
    }
    /**
     * @brief s を接頭辞に持つ文字列の個数 O(|s|)
     */
    template <typename R>
    long long count_prefix(const R& s) const {
        int v = find_node(s);
        return v == -1 ? 0 : nodes[v].prefix_cnt;
    }
    /**
     * @brief 文字列の個数 (重複を含む)
     */
    long long total() const { return nodes[0].prefix_cnt; }

    /**
     * @brief ノード v から文字 c で進んだノード (なければ -1)
     */
    long long child(long long v, int c) const { return nodes[v].next[index(c)]; }
    /**
     * @brief ノード v の親 (根なら -1)
     */
    long long parent(long long v) const { return nodes[v].parent; }
    /**
     * @brief ノード v の深さ (対応する文字列の長さ)
     */
    long long depth(long long v) const { return nodes[v].depth; }
    /**
     * @brief ノード v を通る (v の文字列を接頭辞に持つ) 文字列の個数
     */
    long long count_prefix_at(long long v) const { return nodes[v].prefix_cnt; }
    /**
     * @brief ノード v で終わる文字列の個数
     */
    long long count_at(long long v) const { return nodes[v].word_cnt; }

    /**
     * @brief s の接頭辞のうち、追加した文字列のどれかの接頭辞にもなっているものの最長の長さ O(|s|)
     * 追加した文字列 (取り除いたものは除く) のどれかとの最長共通接頭辞の長さの最大値になる
     */
    template <typename R>
    long long longest_common_prefix(const R& s) const {
        int v = 0;
        long long len = 0;
        for (auto c : s) {
            int u = nodes[v].next[index(c)];
            if (u == -1 || nodes[u].prefix_cnt == 0) break;
            v = u;
            len++;
        }
        return len;
    }
};
