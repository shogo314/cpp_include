#pragma once

/**
 * @file rolling-hash.hpp
 * @brief ローリングハッシュ
 */

#include <cassert>
#include <iterator>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/string/rolling-hash.hpp

struct RHString;
class RollingHash2D;

/**
 * @brief ローリングハッシュ
 *
 * ビット演算による高速化のためハッシュはMOD = 2^61-1で計算される
 *
 * 10^8個くらいなら99%の確率で衝突する組が存在しない
 */
class RollingHash {
    constexpr static unsigned long long MASK30 = (1ULL << 30) - 1;
    constexpr static unsigned long long MASK31 = (1ULL << 31) - 1;
    constexpr static unsigned long long MOD = (1ULL << 61) - 1;

    // a < MOD, b < MOD必須
    constexpr static unsigned long long add(unsigned long long a, unsigned long long b) {
        if ((a += b) >= MOD) a -= MOD;
        return a;
    }

    // a < MOD, b < MOD必須
    constexpr static unsigned long long mul(unsigned long long a, unsigned long long b) {
        __uint128_t c = static_cast<__uint128_t>(a) * b;
        return add(static_cast<unsigned long long>(c >> 61), static_cast<unsigned long long>(c & MOD));
    }

    void expand(long long n) {
        while (static_cast<long long>(power.size()) <= n) power.push_back(mul(power.back(), base));
    }
    // base の逆元の累乗を n 乗まで用意する (RHString の端を削るときだけ使う)
    void expand_inv(long long n) {
        if (inv_power.empty()) {
            // MOD は素数なので base^(MOD - 2) が逆元
            unsigned long long inv = 1, b = base, e = MOD - 2;
            for (; e > 0; e >>= 1, b = mul(b, b))
                if (e & 1) inv = mul(inv, b);
            inv_base = inv;
            inv_power = {1};
        }
        while (static_cast<long long>(inv_power.size()) <= n) inv_power.push_back(mul(inv_power.back(), inv_base));
    }
    unsigned long long inv_base = 0;
    std::vector<unsigned long long> inv_power;  // base の逆元の累乗

   public:
    /**
     * @brief 要素を[0, MOD)の値に変換する
     * @note charは符号に関わらず0~255として扱う 負の整数はMODで正規化する
     */
    template <typename T>
    constexpr static unsigned long long to_value(const T& x) {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, char> || std::is_same_v<U, signed char>) {
            return static_cast<unsigned char>(x);
        } else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
            long long v = static_cast<long long>(x) % static_cast<long long>(MOD);
            return v < 0 ? v + MOD : v;
        } else {
            return static_cast<unsigned long long>(x) % MOD;
        }
    }

    unsigned int base;                      //!< ハッシュの基数
    std::vector<unsigned long long> power;  //!< baseの累乗

    /**
     * @brief コンストラクタ
     *
     * @param base ハッシュの基数 省略するとランダム
     */
    explicit RollingHash(unsigned int base = 0) : base(base) {
        if (base == 0) {
            std::mt19937 mt(std::random_device{}());
            this->base = std::uniform_int_distribution<unsigned int>(129, ~0U)(mt);
        }
        power = {1};
    }

    /**
     * @brief 配列/文字列のイテレータ間のハッシュの計算(O(N))
     *
     * @tparam It イテレータ
     * @param first 配列の開始イテレータ
     * @param last 配列の終了イテレータ
     * @return std::vector<unsigned long long> 先頭から各要素数分のハッシュ
     */
    template <typename It>
    std::vector<unsigned long long> build(It first, It last) {
        std::vector<unsigned long long> res;
        if constexpr (std::is_convertible_v<typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>) {
            res.reserve(last - first + 1);
        }
        res.push_back(0);
        for (; first != last; ++first) {
            res.push_back(add(mul(res.back(), base), to_value(*first)));
        }
        return res;
    }

    /**
     * @brief 配列/文字列全体のハッシュの計算(O(N))
     *
     * @param s 配列/文字列
     * @return std::vector<unsigned long long> 先頭から各文字数分のハッシュ
     */
    template <typename R>
    std::vector<unsigned long long> build(R&& s) {
        using std::begin, std::end;
        return build(begin(s), end(s));
    }

    /**
     * @brief [l,r)のハッシュの計算(O(1))
     *
     * @param hash 先頭からのハッシュ
     * @param l 区間の左端
     * @param r 区間の右端
     * @return unsigned long long [l,r)のハッシュ
     */
    unsigned long long query(const std::vector<unsigned long long>& hash, long long l, long long r) {
        assert(0 <= l && l <= r && r < static_cast<long long>(hash.size()));
        expand(r - l);
        return add(hash[r], MOD - mul(hash[l], power[r - l]));
    }

    friend RHString;
    friend RollingHash2D;
};

/**
 * @brief ローリングハッシュによって管理される文字列型
 */
struct RHString {
    RollingHash& rh;
    long long sz;              //!< 長さ
    unsigned long long hash1;  //!< 正順
    unsigned long long hash2;  //!< 逆順
    /**
     * @brief コンストラクタ
     * 予めRollingHashをインスタンス化しておく必要がある
     */
    explicit RHString(RollingHash& rh) : rh(rh), sz(0), hash1(0), hash2(0) {}
    RHString(RollingHash& rh, long long sz, unsigned long long hash1, unsigned long long hash2) : rh(rh), sz(sz), hash1(hash1), hash2(hash2) {}
    RHString(const RHString& o) : rh(o.rh), sz(o.sz), hash1(o.hash1), hash2(o.hash2) {}
    /**
     * @brief vectorなどで初期化する
     */
    template <class R, ENABLE_IF_T(is_range_v<R> && !std::is_convertible_v<R, std::string_view>)>
    RHString(RollingHash& rh, R&& v) : rh(rh) {
        using std::begin, std::end, std::rbegin, std::rend;
        sz = static_cast<long long>(std::distance(begin(v), end(v)));
        hash1 = rh.build(begin(v), end(v)).back();
        hash2 = rh.build(rbegin(v), rend(v)).back();
    }
    /**
     * @brief charやunsigned long longなどで初期化する
     */
    template <class T, ENABLE_IF_T(std::is_convertible_v<T, unsigned long long> && !std::is_convertible_v<T, std::string_view>)>
    RHString(RollingHash& rh, T&& x) : rh(rh) {
        sz = 1;
        hash1 = RollingHash::to_value(x);
        hash2 = hash1;
    }
    /**
     * @brief 文字列(string, const char*, string_view)で初期化する
     */
    RHString(RollingHash& rh, std::string_view s) : rh(rh) {
        sz = static_cast<long long>(s.size());
        hash1 = rh.build(s.begin(), s.end()).back();
        hash2 = rh.build(s.rbegin(), s.rend()).back();
    }

    /**
     * @brief 回文か否か
     */
    bool is_palindrome() const {
        return hash1 == hash2;
    }
    long long size() const {
        return sz;
    }
    void clear() {
        sz = 0;
        hash1 = 0;
        hash2 = 0;
    }
    bool empty() const {
        return sz == 0;
    }
    /**
     * @brief 先頭の t を取り除く (先頭が t であること。ハッシュからは確かめられないので、違うと値が壊れる) O(1) (累乗の表を伸ばす分を除く)
     * t は RHString のほか、RHString を作れるもの (文字・文字列など) でもよい
     */
    void remove_prefix(const RHString& t) {
        assert(&rh == &t.rh && t.sz <= sz);
        // s = t + u から u: hash1(u) = hash1(s) - hash1(t) B^|u|、hash2(u) = (hash2(s) - hash2(t)) B^-|t|
        const long long u = sz - t.sz;
        rh.expand(u);
        rh.expand_inv(t.sz);
        hash1 = rh.add(hash1, RollingHash::MOD - rh.mul(t.hash1, rh.power[u]));
        hash2 = rh.mul(rh.add(hash2, RollingHash::MOD - t.hash2), rh.inv_power[t.sz]);
        sz = u;
    }
    /**
     * @brief 末尾の t を取り除く (末尾が t であること。ハッシュからは確かめられないので、違うと値が壊れる) O(1) (累乗の表を伸ばす分を除く)
     * t は RHString のほか、RHString を作れるもの (文字・文字列など) でもよい
     */
    void remove_suffix(const RHString& t) {
        assert(&rh == &t.rh && t.sz <= sz);
        // s = u + t から u: hash1(u) = (hash1(s) - hash1(t)) B^-|t|、hash2(u) = hash2(s) - hash2(t) B^|u|
        const long long u = sz - t.sz;
        rh.expand(u);
        rh.expand_inv(t.sz);
        hash1 = rh.mul(rh.add(hash1, RollingHash::MOD - t.hash1), rh.inv_power[t.sz]);
        hash2 = rh.add(hash2, RollingHash::MOD - rh.mul(t.hash2, rh.power[u]));
        sz = u;
    }
    template <class T, ENABLE_IF_T(!std::is_same_v<std::decay_t<T>, RHString>)>
    void remove_prefix(T&& x) {
        remove_prefix(RHString(rh, std::forward<T>(x)));
    }
    template <class T, ENABLE_IF_T(!std::is_same_v<std::decay_t<T>, RHString>)>
    void remove_suffix(T&& x) {
        remove_suffix(RHString(rh, std::forward<T>(x)));
    }
    RHString& operator+=(const RHString& o) {
        assert(&rh == &o.rh);
        rh.expand(sz);
        rh.expand(o.sz);
        hash1 = rh.add(rh.mul(hash1, rh.power[o.sz]), o.hash1);
        hash2 = rh.add(hash2, rh.mul(o.hash2, rh.power[sz]));
        sz += o.sz;
        return *this;
    }
    /**
     * @brief 再代入する
     * RollingHashは同じものである必要がある
     */
    void assign(const RHString& o) {
        assert(&rh == &o.rh);
        sz = o.sz;
        hash1 = o.hash1;
        hash2 = o.hash2;
    }
    /**
     * @brief vectorなどを再代入する
     */
    template <class R, ENABLE_IF_T(is_range_v<R> && !std::is_convertible_v<R, std::string_view>)>
    void assign(R&& v) {
        using std::begin, std::end, std::rbegin, std::rend;
        sz = static_cast<long long>(std::distance(begin(v), end(v)));
        hash1 = rh.build(begin(v), end(v)).back();
        hash2 = rh.build(rbegin(v), rend(v)).back();
    }
    /**
     * @brief charやunsigned long longなどを再代入する
     */
    template <class T, ENABLE_IF_T(std::is_convertible_v<T, unsigned long long> && !std::is_convertible_v<T, std::string_view>)>
    void assign(T&& x) {
        sz = 1;
        hash1 = RollingHash::to_value(x);
        hash2 = hash1;
    }
    /**
     * @brief 文字列(string, const char*, string_view)を再代入する
     */
    void assign(std::string_view s) {
        sz = static_cast<long long>(s.size());
        hash1 = rh.build(s.begin(), s.end()).back();
        hash2 = rh.build(s.rbegin(), s.rend()).back();
    }
    /**
     * @brief 再代入する
     * RollingHashは同じものである必要がある
     */
    RHString& operator=(const RHString& o) {
        assign(o);
        return *this;
    }
    /**
     * @brief vectorなどを再代入する
     */
    template <class R, ENABLE_IF_T(is_range_v<R> && !std::is_convertible_v<R, std::string_view>)>
    RHString& operator=(R&& v) {
        assign(v);
        return *this;
    }
    /**
     * @brief charやunsigned long longなどを再代入する
     */
    template <class T, ENABLE_IF_T(std::is_convertible_v<T, unsigned long long> && !std::is_convertible_v<T, std::string_view>)>
    RHString& operator=(T&& x) {
        assign(x);
        return *this;
    }
    /**
     * @brief 文字列(string, const char*, string_view)を再代入する
     */
    RHString& operator=(std::string_view s) {
        assign(s);
        return *this;
    }
    friend RHString operator+(const RHString& t1, const RHString& t2) {
        RHString ret = t1;
        ret += t2;
        return ret;
    }
    friend bool operator==(const RHString& t1, const RHString& t2) {
        assert(&t1.rh == &t2.rh);
        return t1.sz == t2.sz && t1.hash1 == t2.hash1 && t1.hash2 == t2.hash2;
    }
    friend bool operator!=(const RHString& t1, const RHString& t2) { return !(t1 == t2); }
};

/**
 * @brief 2 次元のローリングハッシュ
 *
 * 行方向と列方向で別の基数を使い、表の左上からの累積ハッシュで長方形のハッシュを O(1) で求める
 * H[i][j] = Σ_{a < i, b < j} v[a][b] base1^(i-1-a) base2^(j-1-b)
 */
class RollingHash2D {
    using ull = unsigned long long;
    RollingHash rh1, rh2;  // 行方向 (上下) と列方向 (左右)

    static ull add(ull a, ull b) { return RollingHash::add(a, b); }
    static ull sub(ull a, ull b) { return RollingHash::add(a, RollingHash::MOD - b); }
    static ull mul(ull a, ull b) { return RollingHash::mul(a, b); }

   public:
    /**
     * @brief コンストラクタ
     *
     * @param base1 行方向の基数 省略するとランダム
     * @param base2 列方向の基数 省略するとランダム
     */
    explicit RollingHash2D(unsigned int base1 = 0, unsigned int base2 = 0) : rh1(base1), rh2(base2) {}

    /**
     * @brief 表 (std::vector<std::string> や std::vector<std::vector<T>>) の累積ハッシュ O(HW)
     *
     * @return std::vector<std::vector<unsigned long long>> (H + 1) 行 (W + 1) 列の表。[i][j] は左上 i 行 j 列のハッシュ
     */
    template <typename Grid>
    std::vector<std::vector<ull>> build(const Grid& grid) {
        using std::size;
        const long long h = size(grid), w = h == 0 ? 0 : size(grid[0]);
        std::vector<std::vector<ull>> res(h + 1, std::vector<ull>(w + 1, 0));
        const ull b1 = rh1.base, b2 = rh2.base, b12 = mul(b1, b2);
        for (long long i = 0; i < h; i++) {
            assert(static_cast<long long>(size(grid[i])) == w);
            for (long long j = 0; j < w; j++) {
                ull v = add(mul(res[i][j + 1], b1), mul(res[i + 1][j], b2));
                res[i + 1][j + 1] = add(sub(v, mul(res[i][j], b12)), RollingHash::to_value(grid[i][j]));
            }
        }
        return res;
    }

    /**
     * @brief 行 [i1, i2)、列 [j1, j2) の長方形のハッシュ O(1) (初回は長さ分)
     */
    ull query(const std::vector<std::vector<ull>>& hash, long long i1, long long j1, long long i2, long long j2) {
        assert(0 <= i1 && i1 <= i2 && i2 < static_cast<long long>(hash.size()));
        assert(0 <= j1 && j1 <= j2 && j2 < static_cast<long long>(hash[0].size()));
        rh1.expand(i2 - i1);
        rh2.expand(j2 - j1);
        ull p1 = rh1.power[i2 - i1], p2 = rh2.power[j2 - j1];
        ull res = sub(hash[i2][j2], mul(hash[i1][j2], p1));
        res = sub(res, mul(hash[i2][j1], p2));
        return add(res, mul(mul(hash[i1][j1], p1), p2));
    }
};

/**
 * @brief 表 text の中で、表 pattern と一致する部分の左上の位置 (行, 列) をすべて求める O(HW + hw)
 *
 * 2 次元のローリングハッシュで比べる。行の昇順、同じ行なら列の昇順に並べる
 * pattern は 1 行 1 列以上であること
 */
template <typename Grid1, typename Grid2>
std::vector<std::pair<long long, long long>> pattern_match_2d(const Grid1& text, const Grid2& pattern) {
    using std::size;
    const long long h = size(text), w = h == 0 ? 0 : size(text[0]);
    const long long ph = size(pattern), pw = ph == 0 ? 0 : size(pattern[0]);
    assert(ph > 0 && pw > 0);
    std::vector<std::pair<long long, long long>> res;
    if (ph > h || pw > w) return res;
    RollingHash2D rh;
    auto th = rh.build(text), pht = rh.build(pattern);
    unsigned long long target = rh.query(pht, 0, 0, ph, pw);
    for (long long i = 0; i + ph <= h; i++) {
        for (long long j = 0; j + pw <= w; j++) {
            if (rh.query(th, i, j, i + ph, j + pw) == target) res.emplace_back(i, j);
        }
    }
    return res;
}
