#pragma once
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/string/palindromic-tree.hpp

/**
 * @file palindromic-tree.hpp
 * @brief 回文木 (Eertree)
 */

#include <array>
#include <cassert>
#include <vector>

/**
 * @brief 回文木 (Eertree, Palindromic Tree)
 *
 * 列の空でない回文の部分列 (種類) をすべて頂点として持つ。末尾に 1 文字ずつ追加でき、全体で O(n) (文字の種類を K として償却 O(nK) のメモリ)
 * 頂点 0 は長さ -1 の根 (ODD)、頂点 1 は長さ 0 の根 (EVEN)。回文の頂点は 2 から、初めて現れた順 (最初に現れたときの終わりの位置の順) に番号が付く
 * 文字 c は c - Base として 0 以上 K 未満の番号に直す
 * 例:
 * PalindromicTree<26, 'a'> pt("abacaba");
 * pt.count_distinct();      // 7 (a, b, c, aba, aca, bacab, abacaba)
 * pt.occurrences()[v];      // 頂点 v の回文の出現回数
 * pt.longest_suffix(6);     // 位置 6 で終わる最長の回文の頂点 (abacaba)
 *
 * @tparam K 文字の種類数
 * @tparam Base 最小の文字
 */
template <int K = 26, int Base = 'a'>
class PalindromicTree {
    struct Node {
        std::array<int, K> next;
        int len;        // 回文の長さ
        int link;       // suffix link (自分より短い最長の回文接尾辞の頂点)
        int parent;     // 両端の 1 文字を除いた回文の頂点
        int cnt;        // この頂点が最長の回文接尾辞になった位置の個数
        int first_end;  // 初めて現れたときの終わりの位置
    };
    // メモリを抑えるため、頂点の中身は int で持つ。外へ返すときに long long にする
    std::vector<Node> nodes;
    std::vector<int> s;        // 追加した文字 (番号に直したもの)
    std::vector<int> longest;  // 各位置で終わる最長の回文の頂点
    int last = 1;              // 今の列の最長の回文接尾辞の頂点

    int add_node(int len, int link, int parent, int first_end) {
        Node node;
        node.next.fill(-1);
        node.len = len, node.link = link, node.parent = parent;
        node.cnt = 0, node.first_end = first_end;
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    static int index(int c) {
        assert(0 <= c - Base && c - Base < K);
        return c - Base;
    }
    // v から suffix link をたどり、位置 i の文字 c を両端に付けられる最初の頂点
    int extendable(int v, int i, int c) const {
        while (true) {
            int j = i - 1 - nodes[v].len;
            if (j >= 0 && s[j] == c) return v;
            v = nodes[v].link;
        }
    }

   public:
    PalindromicTree() {
        add_node(-1, 0, -1, -1);  // ODD (長さ -1、suffix link は自分自身)
        add_node(0, 0, -1, -1);   // EVEN (長さ 0、suffix link は ODD)
    }
    /**
     * @brief 列 s の回文木 O(|s|)
     * @param s std::string や std::vector<int> など、要素を整数に変換できる列
     */
    template <typename R>
    explicit PalindromicTree(const R& s) : PalindromicTree() {
        for (auto c : s) add(c);
    }

    /**
     * @brief 末尾に文字 c を追加し、追加後の列の最長の回文接尾辞の頂点を返す 償却 O(1) (新しい頂点には O(K))
     */
    long long add(int c) {
        c = index(c);
        const int i = s.size();
        s.push_back(c);
        int v = extendable(last, i, c);
        if (nodes[v].next[c] == -1) {
            // 新しい回文 c + (v の回文) + c。suffix link を先に求めてから辺を張る
            int len = nodes[v].len + 2;
            int link = len == 1 ? 1 : nodes[extendable(nodes[v].link, i, c)].next[c];
            int u = add_node(len, link, v, i);
            nodes[v].next[c] = u;
        }
        last = nodes[v].next[c];
        nodes[last].cnt++;
        longest.push_back(last);
        return last;
    }

    /**
     * @brief 頂点の個数 (2 つの根を含む)
     */
    long long size() const { return nodes.size(); }
    /**
     * @brief 異なる空でない回文の個数 (根を除く頂点の個数)
     */
    long long count_distinct() const { return nodes.size() - 2; }
    /**
     * @brief 追加した列の長さ
     */
    long long length() const { return s.size(); }

    /**
     * @brief 頂点 v の回文の長さ (ODD は -1、EVEN は 0)
     */
    long long len(long long v) const { return nodes[v].len; }
    /**
     * @brief 頂点 v の suffix link (自分より短い最長の空でない回文接尾辞の頂点。なければ EVEN = 1)
     * 根では、ODD は 0 (自分自身)、EVEN は 0 (ODD)
     */
    long long suffix_link(long long v) const { return nodes[v].link; }
    /**
     * @brief 頂点 v の親 (両端の 1 文字を除いた回文の頂点。長さ 1 なら ODD = 0、長さ 2 なら EVEN = 1、根なら -1)
     */
    long long parent(long long v) const { return nodes[v].parent; }
    /**
     * @brief 頂点 v の回文の両端に文字 c を付けた回文の頂点 (なければ -1)
     */
    long long child(long long v, int c) const { return nodes[v].next[index(c)]; }
    /**
     * @brief 頂点 v の回文が初めて現れたときの終わりの位置 (0-indexed、根は -1)
     * 回文は [first_end(v) - len(v) + 1, first_end(v)] に現れる
     */
    long long first_end(long long v) const { return nodes[v].first_end; }
    /**
     * @brief 位置 i (0-indexed) で終わる最長の回文の頂点
     */
    long long longest_suffix(long long i) const {
        assert(0 <= i && i < static_cast<long long>(longest.size()));
        return longest[i];
    }

    /**
     * @brief 各頂点の回文が列に現れる回数 (根は 0) O(size())
     * 頂点 v が最長の回文接尾辞になった回数を、suffix link の木で葉の側から足し合わせる
     */
    std::vector<long long> occurrences() const {
        std::vector<long long> res(nodes.size());
        for (int v = 0; v < static_cast<int>(nodes.size()); v++) res[v] = nodes[v].cnt;
        // suffix link の先は自分より前に作った頂点なので、番号の大きい順に足せばよい
        for (int v = static_cast<int>(nodes.size()) - 1; v >= 2; v--) res[nodes[v].link] += res[v];
        res[0] = res[1] = 0;
        return res;
    }
};
