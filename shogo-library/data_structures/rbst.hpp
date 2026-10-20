#pragma once

/**
 * @file rbst.hpp
 * @brief 平衡二分木 (RBST) による列
 */

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/data_structures/rbst.hpp

namespace internal_rbst {
template <typename M, typename = void>
struct has_acted : std::false_type {};
template <typename M>
struct has_acted<M, std::void_t<typename M::F>> : std::true_type {};
// 作用素付きでないモノイドのときの作用素の型
struct NoAct {
    using F = NoAct;
};
}  // namespace internal_rbst

/**
 * @brief 平衡二分木 (RBST) による列
 *
 * 位置で扱う列。k 番目への挿入・削除、区間積、区間反転、split・merge を期待 O(log n) で行う
 * モノイド (algebra/monoid.hpp) か作用素付きモノイド (algebra/acted-monoid.hpp) を乗せる。作用素付きなら区間作用もできる
 * 作用素付きモノイドの mapping は (f, x) か (f, x, sz) の形にする (列の位置は挿入・削除で変わるので (f, x, l, r) は使えない)
 * 頂点は型ごとに共有する配列に置くので、同じ型の木どうしで split・merge できる
 * 例:
 * RBST<acted_monoid::AddSum<long long>> t(v);
 * t.insert(2, x); t.erase(0); t.reverse(1, 4); t.apply(0, 3, 10); t.prod(1, 3);
 * RBST<acted_monoid::AddSum<long long>> u = t.split(2);  // t は先頭 2 個、u は残り
 * t.merge(std::move(u));                                 // 元に戻す
 *
 * @tparam M モノイドか作用素付きモノイド
 */
template <typename M>
class RBST {
   public:
    using S = typename M::S;
    static constexpr bool acted = internal_rbst::has_acted<M>::value;
    using F = typename std::conditional_t<acted, M, internal_rbst::NoAct>::F;

   private:
    static_assert(!acted || !internal_acted_monoid::has_mapping_lr<M>::value, "RBST: mapping(f, x, l, r) は使えない");

    // 頂点 0 は空の木を表す
    struct Node {
        S val, sum, rsum;  // 値、部分木の積、部分木の逆順の積
        F lz;              // 子に伝える作用素
        int sz = 0, l = 0, r = 0;
        bool has_lz = false, rev = false;  // 子に作用素・反転を伝えていないか
    };
    // 頂点の配列は最初に頂点を作るときに確保し、解放しない
    // (グローバル変数の木のデストラクタが、先に破棄された配列を触らないようにする)
    inline static std::vector<Node>* nodes_ = nullptr;
    inline static std::vector<int>* unused_ = nullptr;  // 消した頂点
    static Node& node(int t) { return (*nodes_)[t]; }
    static std::uint32_t rand() {
        static std::uint32_t x = 2463534242U;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
    }

    M m;
    int root = 0;

    S op(const S& x, const S& y) const { return m.op(x, y); }
    S e() const { return m.e(); }
    static int size_of(int t) { return t ? node(t).sz : 0; }
    S sum_of(int t) const { return t ? node(t).sum : e(); }
    S rsum_of(int t) const { return t ? node(t).rsum : e(); }

    // x は値で受け取る (頂点の配列の要素を渡されたとき、配列が伸びて参照が無効になるのを防ぐ)
    int new_node(S x) {
        if (!nodes_) nodes_ = new std::vector<Node>(1), unused_ = new std::vector<int>();
        int t;
        if (!unused_->empty()) {
            t = unused_->back();
            unused_->pop_back();
        } else {
            t = nodes_->size();
            nodes_->emplace_back();
        }
        Node& v = node(t);
        v.val = v.sum = v.rsum = x;
        v.has_lz = v.rev = false;
        v.sz = 1, v.l = v.r = 0;
        return t;
    }
    // 部分木の頂点をすべて消す
    static void free_tree(int t) {
        if (!t) return;
        std::vector<int> st = {t};
        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            if (node(v).l) st.push_back(node(v).l);
            if (node(v).r) st.push_back(node(v).r);
            unused_->push_back(v);
        }
    }
    int clone_tree(int t) {
        if (!t) return 0;
        push(t);
        int lc = clone_tree(node(t).l);
        int c = new_node(node(t).val);
        int rc = clone_tree(node(t).r);
        node(c).l = lc, node(c).r = rc;
        update(c);
        return c;
    }

    void update(int t) {
        Node& v = node(t);
        v.sz = node(v.l).sz + node(v.r).sz + 1;
        v.sum = op(op(sum_of(v.l), v.val), sum_of(v.r));
        v.rsum = op(op(rsum_of(v.r), v.val), rsum_of(v.l));
    }
    void all_apply(int t, const F& f) {
        if (!t) return;
        if constexpr (acted) {
            Node& v = node(t);
            v.val = acted_monoid::apply_mapping(m, f, v.val, 0, 1);
            v.sum = acted_monoid::apply_mapping(m, f, v.sum, 0, v.sz);
            v.rsum = acted_monoid::apply_mapping(m, f, v.rsum, 0, v.sz);
            v.lz = v.has_lz ? m.composition(f, v.lz) : f;
            v.has_lz = true;
        }
    }
    static void toggle(int t) {
        if (!t) return;
        Node& v = node(t);
        std::swap(v.l, v.r);
        std::swap(v.sum, v.rsum);
        v.rev = !v.rev;
    }
    void push(int t) {
        Node& v = node(t);
        if (v.rev) {
            toggle(v.l), toggle(v.r);
            v.rev = false;
        }
        if constexpr (acted) {
            if (v.has_lz) {
                all_apply(v.l, v.lz), all_apply(v.r, v.lz);
                v.has_lz = false;
            }
        }
    }

    int merge_node(int a, int b) {
        if (!a || !b) return a ^ b;
        // 大きさに比例する確率で根を選ぶ ([0, 大きさの和) の乱数を、割り算の代わりに掛け算とシフトで作る)
        if ((static_cast<std::uint64_t>(rand()) * (static_cast<std::uint64_t>(node(a).sz) + node(b).sz) >> 32) < static_cast<std::uint64_t>(node(a).sz)) {
            push(a);
            int c = merge_node(node(a).r, b);
            node(a).r = c;
            update(a);
            return a;
        }
        push(b);
        int c = merge_node(a, node(b).l);
        node(b).l = c;
        update(b);
        return b;
    }
    // 先頭 k 個と残りに分ける
    std::pair<int, int> split_node(int t, int k) {
        if (!t) return {0, 0};
        push(t);
        int ls = node(node(t).l).sz;
        if (k <= ls) {
            auto [a, b] = split_node(node(t).l, k);
            node(t).l = b;
            update(t);
            return {a, t};
        }
        auto [a, b] = split_node(node(t).r, k - ls - 1);
        node(t).r = a;
        update(t);
        return {t, b};
    }
    int build(const std::vector<S>& v, int lo, int hi) {
        if (lo >= hi) return 0;
        int mid = (lo + hi) / 2;
        int lc = build(v, lo, mid);
        int t = new_node(v[mid]);
        int rc = build(v, mid + 1, hi);
        node(t).l = lc, node(t).r = rc;
        update(t);
        return t;
    }
    // [l, r) を取り出して f を作用させ、元に戻す
    template <typename Fn>
    auto with_range(long long l, long long r, Fn fn) {
        assert(0 <= l && l <= r && r <= size());
        auto [a, bc] = split_node(root, static_cast<int>(l));
        auto [b, c] = split_node(bc, static_cast<int>(r - l));
        if constexpr (std::is_void_v<decltype(fn(b))>) {
            fn(b);
            root = merge_node(merge_node(a, b), c);
        } else {
            auto res = fn(b);
            root = merge_node(merge_node(a, b), c);
            return res;
        }
    }
    void to_vector(int t, std::vector<S>& res) {
        if (!t) return;
        push(t);
        to_vector(node(t).l, res);
        res.push_back(node(t).val);
        to_vector(node(t).r, res);
    }

    class RBSTReference {
        RBST& t;
        long long k;

       public:
        RBSTReference(RBST& t, long long k) : t(t), k(k) {}
        RBSTReference& operator=(const S& x) {
            t.set(k, x);
            return *this;
        }
        operator S() { return t.get(k); }
    };

    struct FromRoot {};
    RBST(int root, const M& m, FromRoot) : m(m), root(root) {}

   public:
    /**
     * @brief 空の列
     */
    explicit RBST(M m = M()) : m(std::move(m)) {}
    /**
     * @brief 長さ n で、すべて単位元の列
     */
    explicit RBST(long long n, M m = M()) : RBST(std::vector<S>(n, m.e()), m) {}
    /**
     * @brief v の列 O(n)
     */
    explicit RBST(const std::vector<S>& v, M m = M()) : m(std::move(m)) { root = build(v, 0, static_cast<int>(v.size())); }
    /**
     * @brief 複製する O(n)
     */
    RBST(const RBST& o) : m(o.m) { root = const_cast<RBST&>(o).clone_tree(o.root); }
    RBST(RBST&& o) noexcept : m(std::move(o.m)), root(o.root) { o.root = 0; }
    RBST& operator=(const RBST& o) {
        if (this != &o) {
            int c = const_cast<RBST&>(o).clone_tree(o.root);
            free_tree(root);
            m = o.m, root = c;
        }
        return *this;
    }
    RBST& operator=(RBST&& o) noexcept {
        if (this != &o) {
            free_tree(root);
            m = std::move(o.m), root = o.root;
            o.root = 0;
        }
        return *this;
    }
    ~RBST() { free_tree(root); }

    /**
     * @brief 列の長さ
     */
    long long size() const { return size_of(root); }
    /**
     * @brief 空か
     */
    bool empty() const { return root == 0; }

    /**
     * @brief k 番目の値
     */
    S get(long long k) {
        assert(0 <= k && k < size());
        int t = root;
        while (true) {
            push(t);
            int ls = node(node(t).l).sz;
            if (k < ls) {
                t = node(t).l;
            } else if (k == ls) {
                return node(t).val;
            } else {
                k -= ls + 1;
                t = node(t).r;
            }
        }
    }
    /**
     * @brief k 番目の要素への参照 (代入すると set が呼ばれる)
     */
    RBSTReference operator[](long long k) { return RBSTReference(*this, k); }
    /**
     * @brief k 番目の値を x にする
     */
    void set(long long k, const S& x) {
        assert(0 <= k && k < size());
        with_range(k, k + 1, [&](int t) { node(t).val = node(t).sum = node(t).rsum = x; });
    }
    /**
     * @brief k 番目 (0 <= k <= size()) に x を挿入する。後ろの要素は 1 つずつずれる
     */
    void insert(long long k, const S& x) {
        assert(0 <= k && k <= size());
        auto [a, b] = split_node(root, static_cast<int>(k));
        root = merge_node(merge_node(a, new_node(x)), b);
    }
    void push_front(const S& x) { insert(0, x); }
    void push_back(const S& x) { insert(size(), x); }
    /**
     * @brief k 番目を削除して、その値を返す
     */
    S erase(long long k) {
        assert(0 <= k && k < size());
        auto [a, bc] = split_node(root, static_cast<int>(k));
        auto [b, c] = split_node(bc, 1);
        S x = node(b).val;
        free_tree(b);
        root = merge_node(a, c);
        return x;
    }
    S pop_front() { return erase(0); }
    S pop_back() { return erase(size() - 1); }

    /**
     * @brief [l, r) の積
     */
    S prod(long long l, long long r) {
        return with_range(l, r, [&](int t) { return sum_of(t); });
    }
    /**
     * @brief 列全体の積 O(1)
     */
    S all_prod() const { return sum_of(root); }
    /**
     * @brief [l, r) を反転する
     */
    void reverse(long long l, long long r) {
        with_range(l, r, [&](int t) { toggle(t); });
    }
    /**
     * @brief [l, r) の各値に f を作用させる (作用素付きモノイドのとき)
     */
    template <bool B = acted, std::enable_if_t<B, std::nullptr_t> = nullptr>
    void apply(long long l, long long r, const F& f) {
        with_range(l, r, [&](int t) { all_apply(t, f); });
    }
    /**
     * @brief k 番目の値に f を作用させる (作用素付きモノイドのとき)
     */
    template <bool B = acted, std::enable_if_t<B, std::nullptr_t> = nullptr>
    void apply(long long k, const F& f) {
        assert(0 <= k && k < size());
        apply(k, k + 1, f);
    }

    /**
     * @brief 末尾に other をつなげる。other は空になる
     */
    void merge(RBST&& other) {
        root = merge_node(root, other.root);
        other.root = 0;
    }
    /**
     * @brief 先頭 k 個を残し、残りを切り取って返す
     */
    RBST split(long long k) {
        assert(0 <= k && k <= size());
        auto [a, b] = split_node(root, static_cast<int>(k));
        root = a;
        return RBST(b, m, FromRoot{});
    }

    /**
     * @brief g(prod(l, r)) が true となる最大の r (g(e()) は true であること)
     * g が単調なら、g(prod(l, r)) が true となる r は l から始まる区間をなす
     */
    template <typename G>
    long long max_right(long long l, G g) {
        assert(0 <= l && l <= size());
        assert(g(e()));
        auto [a, b] = split_node(root, static_cast<int>(l));
        long long res = l;
        S acc = e();
        int t = b;
        while (t) {
            push(t);
            S s = op(acc, sum_of(node(t).l));
            if (!g(s)) {
                t = node(t).l;
                continue;
            }
            S s2 = op(s, node(t).val);
            if (!g(s2)) {
                res += node(node(t).l).sz;
                break;
            }
            acc = s2;
            res += node(node(t).l).sz + 1;
            t = node(t).r;
        }
        root = merge_node(a, b);
        return res;
    }
    /**
     * @brief g(prod(l, r)) が true となる最小の l (g(e()) は true であること)
     */
    template <typename G>
    long long min_left(long long r, G g) {
        assert(0 <= r && r <= size());
        assert(g(e()));
        auto [a, b] = split_node(root, static_cast<int>(r));
        long long res = r;
        S acc = e();
        int t = a;
        while (t) {
            push(t);
            S s = op(sum_of(node(t).r), acc);
            if (!g(s)) {
                t = node(t).r;
                continue;
            }
            S s2 = op(node(t).val, s);
            if (!g(s2)) {
                res -= node(node(t).r).sz;
                break;
            }
            acc = s2;
            res -= node(node(t).r).sz + 1;
            t = node(t).l;
        }
        root = merge_node(a, b);
        return res;
    }

    /**
     * @brief 列を std::vector にする O(n)
     */
    std::vector<S> to_vector() {
        std::vector<S> res;
        res.reserve(size());
        to_vector(root, res);
        return res;
    }
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, RBST& t) {
        std::vector<S> v = t.to_vector();
        for (int i = 0; i < static_cast<int>(v.size()); i++) {
            if (i != 0) os << CharT(' ');
            os << v[i];
        }
        return os;
    }
};
