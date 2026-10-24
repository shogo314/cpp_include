#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/string/aho-corasick.hpp

/**
 * @file aho-corasick.hpp
 * @brief Aho-Corasick
 */

#include <array>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

/**
 * @brief Aho-Corasick (複数のパターンの照合)
 *
 * パターンを add で追加してから build を呼ぶ。文字 c は c - Base として 0 以上 K 未満の番号に直す
 * ノードは Trie と同じく、文字列を追加した順に作った番号を持つ。根はノード 0 (空文字列)
 * 例:
 * AhoCorasick<26, 'a'> ac;
 * ac.add("he");
 * ac.add("she");
 * ac.build();
 * ac.count("ushers");  // 2 ("she" と "he")
 *
 * @tparam K 文字の種類数
 * @tparam Base 最小の文字
 */
template <int K = 26, int Base = 'a'>
class AhoCorasick {
    struct Node {
        std::array<int, K> next;  // build 前は Trie の子 (なければ -1)、build 後は文字を読んだときの遷移先
        int parent, depth;
        int link;         // 失敗リンク (このノードの文字列の、最長の真の接尾辞のノード)
        int out;          // 失敗リンクをたどって最初に着く、パターンが終わるノード (なければ -1)
        int head;         // このノードで終わるパターンの番号のリストの先頭 (なければ -1)
        int match_count;  // このノードの文字列の接尾辞になっているパターンの個数
    };
    // メモリを抑えるため、ノードの中身は int で持つ。外へ返すときに long long にする
    std::vector<Node> nodes;
    std::vector<int> pattern_node, next_pattern;  // パターンの終わるノードと、同じノードで終わる次のパターン
    std::vector<int> bfs_order;
    bool built = false;

    int add_node(int parent, int depth) {
        Node node;
        node.next.fill(-1);
        node.parent = parent;
        node.depth = depth;
        node.link = 0;
        node.out = node.head = -1;
        node.match_count = 0;
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    static int index(int c) {
        assert(0 <= c - Base && c - Base < K);
        return c - Base;
    }

   public:
    AhoCorasick() { add_node(-1, 0); }

    /**
     * @brief パターン s を追加し、パターンの番号 (追加した順に 0, 1, ...) を返す O(|s|)
     * 空でない文字列に限る。同じパターンを何度も追加でき、別のパターンとして数える
     * @param s std::string や std::vector<int> など、要素を整数に変換できる列
     */
    template <typename R>
    long long add(const R& s) {
        assert(!built);
        assert(std::begin(s) != std::end(s));
        int v = 0;
        for (auto c : s) {
            int i = index(c);
            if (nodes[v].next[i] == -1) {
                int u = add_node(v, nodes[v].depth + 1);
                nodes[v].next[i] = u;
            }
            v = nodes[v].next[i];
        }
        int id = pattern_node.size();
        pattern_node.push_back(v);
        next_pattern.push_back(nodes[v].head);
        nodes[v].head = id;
        return id;
    }

    /**
     * @brief 失敗リンクと遷移を作る。パターンを追加し終えてから 1 回だけ呼ぶ O(K * ノード数)
     */
    void build() {
        assert(!built);
        built = true;
        for (int v = 0; v < static_cast<int>(nodes.size()); v++) {
            // 同じノードで終わるパターンのリストを、追加した順に並べ直す
            int rev = -1;
            for (int id = nodes[v].head; id != -1;) {
                int nx = next_pattern[id];
                next_pattern[id] = rev;
                rev = id;
                id = nx;
                nodes[v].match_count++;
            }
            nodes[v].head = rev;
        }
        bfs_order.reserve(nodes.size());
        bfs_order.push_back(0);
        for (int i = 0; i < static_cast<int>(bfs_order.size()); i++) {
            int v = bfs_order[i];
            for (int c = 0; c < K; c++) {
                int u = nodes[v].next[c];
                // まだ -1 なら Trie の子はない。失敗リンク先からの遷移を使う
                if (u == -1) {
                    nodes[v].next[c] = v == 0 ? 0 : nodes[nodes[v].link].next[c];
                    continue;
                }
                int l = v == 0 ? 0 : nodes[nodes[v].link].next[c];
                nodes[u].link = l;
                nodes[u].out = nodes[l].head != -1 ? l : nodes[l].out;
                nodes[u].match_count += nodes[l].match_count;
                bfs_order.push_back(u);
            }
        }
    }

    /**
     * @brief ノードの個数 (根を含む)
     */
    long long size() const { return nodes.size(); }
    /**
     * @brief パターンの個数
     */
    long long pattern_count() const { return pattern_node.size(); }
    /**
     * @brief パターン i が終わるノード
     */
    long long node(long long i) const {
        assert(0 <= i && i < pattern_count());
        return pattern_node[i];
    }
    /**
     * @brief ノード v の親 (根なら -1)
     */
    long long parent(long long v) const { return nodes[v].parent; }
    /**
     * @brief ノード v の深さ (対応する文字列の長さ)
     */
    long long depth(long long v) const { return nodes[v].depth; }

    /**
     * @brief ノード v の失敗リンク (v の文字列の最長の真の接尾辞のうち、ノードのあるもの)。根なら -1
     */
    long long link(long long v) const {
        assert(built);
        return v == 0 ? -1 : nodes[v].link;
    }
    /**
     * @brief ノード v で文字 c を読んだときに移るノード
     */
    long long next(long long v, int c) const {
        assert(built);
        return nodes[v].next[index(c)];
    }
    /**
     * @brief ノード v の文字列の接尾辞になっているパターンの個数 (v に着いたときに終わるパターンの個数)
     */
    long long match_count(long long v) const {
        assert(built);
        return nodes[v].match_count;
    }

    /**
     * @brief text に現れるパターンの個数の合計 (同じパターンが何回現れても、すべて数える) O(|text|)
     */
    template <typename R>
    long long count(const R& text) const {
        assert(built);
        long long res = 0;
        int v = 0;
        for (auto c : text) {
            v = nodes[v].next[index(c)];
            res += nodes[v].match_count;
        }
        return res;
    }
    /**
     * @brief パターンごとに、text に現れる回数を返す O(|text| + ノード数 + パターン数)
     */
    template <typename R>
    std::vector<long long> count_each(const R& text) const {
        assert(built);
        std::vector<long long> visit(nodes.size());
        int v = 0;
        for (auto c : text) {
            v = nodes[v].next[index(c)];
            visit[v]++;
        }
        // 失敗リンクの木で、葉の方から回数を足し上げる
        for (int i = static_cast<int>(bfs_order.size()) - 1; i > 0; i--) {
            int u = bfs_order[i];
            visit[nodes[u].link] += visit[u];
        }
        std::vector<long long> res(pattern_node.size());
        for (int i = 0; i < static_cast<int>(pattern_node.size()); i++) res[i] = visit[pattern_node[i]];
        return res;
    }
    /**
     * @brief text に現れるパターンをすべて (開始位置, パターンの番号) の組で返す O(|text| + 出力の個数)
     * 終わる位置の順に並び、同じ位置で終わるものは長いパターンから、同じパターンは番号の小さい順に並ぶ
     */
    template <typename R>
    std::vector<std::pair<long long, long long>> find_all(const R& text) const {
        assert(built);
        std::vector<std::pair<long long, long long>> res;
        int v = 0;
        long long pos = 0;
        for (auto c : text) {
            v = nodes[v].next[index(c)];
            pos++;
            for (int u = nodes[v].head != -1 ? v : nodes[v].out; u != -1; u = nodes[u].out) {
                for (int id = nodes[u].head; id != -1; id = next_pattern[id]) res.emplace_back(pos - nodes[u].depth, id);
            }
        }
        return res;
    }
};
