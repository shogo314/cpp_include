#pragma once
#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

/**
 * infをもつ整数型
 * Tは符号付き整数であり、2の補数表現で表されることを期待する
 */
template <typename T = int, std::enable_if_t<std::is_integral_v<T> and std::is_signed_v<T>, std::nullptr_t> = nullptr>
struct infint {
   private:
    static_assert(std::numeric_limits<T>::min() + 1 == -std::numeric_limits<T>::max());

    T _v;

    inline constexpr static T _PlusInf = std::numeric_limits<T>::max();       //<! 正の無限大
    inline constexpr static T _MinusInf = std::numeric_limits<T>::min() + 1;  //<! 負の無限大
    inline constexpr static T _NaN = std::numeric_limits<T>::min();           //<! Not a Number

   public:
    constexpr infint() noexcept : _v(0) {}
    constexpr infint(const T& t) noexcept : _v(t) {}
    constexpr infint(const infint& o) noexcept : _v(o._v) {}
    infint(std::string_view sv) noexcept(false) {
        std::string s(sv);
        std::for_each(s.begin(), s.end(), [](char& c) {
            if ('A' <= c and c <= 'Z') {
                c = c - 'A' + 'a';
            }
        });
        if (s == "plusinf") {
            _v = _PlusInf;
        } else if (s == "inf") {
            _v = _PlusInf;
        } else if (s == "minusinf") {
            _v = _MinusInf;
        } else if (s == "nan") {
            _v = _NaN;
        } else {
            _v = std::stoll(s);
        }
    }

    /**
     * 内部表現をそのまま返す
     */
    constexpr T val() const noexcept { return _v; }

    constexpr bool isNaN() const noexcept { return _v == _NaN; }
    constexpr bool isPlusInf() const noexcept { return _v == _PlusInf; }
    constexpr bool isMinusInf() const noexcept { return _v == _MinusInf; }
    constexpr bool isInf() const noexcept { return _v == _PlusInf; }
    constexpr bool isfinite() const noexcept { return _MinusInf < _v and _v < _PlusInf; }

    constexpr infint& operator=(const infint& o) noexcept {
        _v = o._v;
        return *this;
    }
    constexpr infint& operator=(const T& t) noexcept {
        _v = t;
        return *this;
    }

    constexpr infint operator+() const noexcept { return _v; }
    constexpr infint operator-() const noexcept {
        if (_v == _NaN) return _v;
        return -_v;
    }
    constexpr infint& operator*=(const infint& o) noexcept {
        if (isNaN() or o.isNaN()) {
            _v = _NaN;
            return *this;
        }
        if (not isfinite() or not o.isfinite()) {
            if (_v == 0 or o._v == 0) {
                _v = _NaN;
            } else if ((_v > 0) ^ (o._v > 0)) {
                _v = _MinusInf;
            } else {
                _v = _PlusInf;
            }
            return *this;
        }
        if (_v == 0 or o._v == 0) {
            _v = 0;
            return *this;
        }
        if (std::abs(_v) >= _PlusInf / std::abs(o._v)) {
            if ((_v > 0) ^ (o._v > 0)) {
                _v = _MinusInf;
            } else {
                _v = _PlusInf;
            }
            return *this;
        }
        _v *= o._v;
        return *this;
    }
    constexpr infint& operator/=(const infint& o) noexcept {
        if (isNaN() or o.isNaN()) {
            _v = _NaN;
            return *this;
        }
        if (not isfinite() and not o.isfinite()) {
            _v = _NaN;
            return *this;
        }
        if (o._v == 0) {
            _v = _NaN;
            return *this;
        }
        if (not o.isfinite()) {
            _v = 0;
            return *this;
        }
        _v /= o._v;
        return *this;
    }
    constexpr infint& operator%=(const infint& o) noexcept {
        if (isNaN() or o.isNaN()) {
            _v = _NaN;
            return *this;
        }
        if (not isfinite() and not o.isfinite()) {
            _v = _NaN;
            return *this;
        }
        if (o._v == 0) {
            _v = _NaN;
            return *this;
        }
        if (not o.isfinite()) {
            return *this;
        }
        _v %= o._v;
        return *this;
    }
    constexpr infint& operator+=(const infint& o) noexcept {
        if (isNaN() or o.isNaN()) {
            _v = _NaN;
            return *this;
        }
        if (not isfinite() or not o.isfinite()) {
            if (not isfinite() and not o.isfinite()) {
                if (isPlusInf() ^ o.isPlusInf()) {
                    _v = _NaN;
                }
            } else if (not o.isfinite()) {
                _v = o._v;
            }
            return *this;
        }
        if (o._v > 0) {
            if (_v >= _PlusInf - o._v) {
                _v = _PlusInf;
                return *this;
            }
        } else if (o._v < 0) {
            if (_v <= _MinusInf - o._v) {
                _v = _MinusInf;
                return *this;
            }
        }
        _v += o._v;
        return *this;
    }
    constexpr infint& operator-=(const infint& o) noexcept {
        if (isNaN() or o.isNaN()) {
            _v = _NaN;
            return *this;
        }
        if (not isfinite() or not o.isfinite()) {
            if (not isfinite() and not o.isfinite()) {
                if (isPlusInf() == o.isPlusInf()) {
                    _v = _NaN;
                }
            } else if (not o.isfinite()) {
                _v = -o._v;
            }
            return *this;
        }
        if (o._v > 0) {
            if (_v <= _MinusInf + o._v) {
                _v = _MinusInf;
                return *this;
            }
        } else if (o._v < 0) {
            if (_v >= _PlusInf + o._v) {
                _v = _PlusInf;
                return *this;
            }
        }
        _v -= o._v;
        return *this;
    }
    static std::string type_str() { return "infint<>"; }
    std::string initializer_str() const {
        if (_v == _PlusInf) {
            return "{\"plusinf\"}";
        } else if (_v == _MinusInf) {
            return "{\"minusinf\"}";
        } else if (_v == _NaN) {
            return "{\"nan\"}";
        } else {
            return "{" + std::to_string(_v) + "}";
        }
    }
    std::string repr() const {
        return this->type_str() + this->initializer_str();
    }

    /**
     * 正の無限大
     */
    constexpr static infint PlusInf() noexcept {
        return _PlusInf;
    }
    /**
     * 負の無限大
     */
    constexpr static infint MinusInf() noexcept {
        return _MinusInf;
    }
    /**
     * Not a Number
     */
    constexpr static infint NaN() noexcept {
        return _NaN;
    }
    /**
     * 正の無限大(PlusInfと同じ)
     */
    constexpr static infint Inf() noexcept {
        return _PlusInf;
    }
    /**
     * 正の無限大でない最大値
     */
    constexpr static infint Max() noexcept {
        return _PlusInf - 1;
    }
    /**
     * 負の無限大でない最小値
     */
    constexpr static infint Min() noexcept {
        return _MinusInf + 1;
    }

    friend constexpr infint operator*(const infint& t1, const infint& t2) noexcept {
        infint res(t1);
        return res *= t2;
    }
    friend constexpr infint operator/(const infint& t1, const infint& t2) noexcept {
        infint res(t1);
        return res /= t2;
    }
    friend constexpr infint operator%(const infint& t1, const infint& t2) noexcept {
        infint res(t1);
        return res %= t2;
    }
    friend constexpr infint operator+(const infint& t1, const infint& t2) noexcept {
        infint res(t1);
        return res += t2;
    }
    friend constexpr infint operator-(const infint& t1, const infint& t2) noexcept {
        infint res(t1);
        return res -= t2;
    }
    friend constexpr bool operator<(const infint& t1, const infint& t2) noexcept {
        if (t1.isNaN() or t2.isNaN()) return false;
        return t1._v < t2._v;
    }
    friend constexpr bool operator>(const infint& t1, const infint& t2) noexcept {
        if (t1.isNaN() or t2.isNaN()) return false;
        return t1._v > t2._v;
    }
    friend constexpr bool operator<=(const infint& t1, const infint& t2) noexcept {
        if (t1.isNaN() or t2.isNaN()) return false;
        return t1._v <= t2._v;
    }
    friend constexpr bool operator>=(const infint& t1, const infint& t2) noexcept {
        if (t1.isNaN() or t2.isNaN()) return false;
        return t1._v >= t2._v;
    }
    friend constexpr bool operator==(const infint& t1, const infint& t2) noexcept {
        return t1._v == t2._v;
    }
    friend constexpr bool operator!=(const infint& t1, const infint& t2) noexcept {
        return t1._v != t2._v;
    }
};
