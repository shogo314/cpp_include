#pragma once
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/utility/compress.hpp

/**
 * @file compress.hpp
 * @brief 座標圧縮
 */

#include <algorithm>
#include <cassert>
#include <vector>

/**
 * @brief 座標圧縮
 *
 * 値を add で登録し、build でソートと重複除去をしてから使う
 * 例:
 * Compress<long long> cp(A);  // A を登録して build まで行う
 * long long i = cp.id(A[0]);  // 0 <= i < cp.size()
 *
 * @tparam T 値の型 (< で比較できること)
 */
template <typename T>
class Compress {
    std::vector<T> xs;
    bool built = false;

   public:
    Compress() = default;
    /**
     * @brief v の値を登録して build する
     */
    explicit Compress(const std::vector<T>& v) : xs(v) { build(); }

    /**
     * @brief 値を 1 つ登録する。build の前に呼ぶ
     */
    void add(const T& x) {
        assert(!built);
        xs.push_back(x);
    }
    /**
     * @brief v の値をすべて登録する。build の前に呼ぶ
     */
    void add(const std::vector<T>& v) {
        assert(!built);
        xs.insert(xs.end(), v.begin(), v.end());
    }
    /**
     * @brief 登録した値をソートして重複を除く O(n log n)
     */
    void build() {
        assert(!built);
        std::sort(xs.begin(), xs.end());
        xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
        built = true;
    }

    /**
     * @brief 異なる値の個数
     */
    long long size() const {
        assert(built);
        return xs.size();
    }
    /**
     * @brief 登録した値 x の番号 O(log n)。登録していない値なら assert で落ちる
     */
    long long id(const T& x) const {
        const long long i = bisect_left(x);
        assert(i < size() && !(x < xs[i]));
        return i;
    }
    /**
     * @brief x 以上で最小の値の番号 O(log n)。なければ size()
     */
    long long bisect_left(const T& x) const {
        assert(built);
        return std::lower_bound(xs.begin(), xs.end(), x) - xs.begin();
    }
    /**
     * @brief x より大きい最小の値の番号 O(log n)。なければ size()
     */
    long long bisect_right(const T& x) const {
        assert(built);
        return std::upper_bound(xs.begin(), xs.end(), x) - xs.begin();
    }
    /**
     * @brief x を登録したか O(log n)
     */
    bool contains(const T& x) const {
        const long long i = bisect_left(x);
        return i < size() && !(x < xs[i]);
    }
    /**
     * @brief 番号 i の値
     */
    const T& operator[](long long i) const {
        assert(built && 0 <= i && i < size());
        return xs[i];
    }
    /**
     * @brief v の各値を番号に置き換えた列
     */
    std::vector<long long> ids(const std::vector<T>& v) const {
        std::vector<long long> res(v.size());
        for (long long i = 0; i < (long long)v.size(); i++) res[i] = id(v[i]);
        return res;
    }
    /**
     * @brief ソート済みで重複のない値の列
     */
    const std::vector<T>& values() const {
        assert(built);
        return xs;
    }
};
