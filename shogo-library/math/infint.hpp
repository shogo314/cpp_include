#pragma once
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/math/infint.hpp

/**
 * @file infint.hpp
 * @brief inf を持つ整数型
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

/**
 * infをもつ整数型
 * Tは符号付き整数であり、2の補数表現で表されることを期待する
 */
template <typename T = long long>
class infint {
   private:
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
    static_assert(std::numeric_limits<T>::min() + 1 == -std::numeric_limits<T>::max());

    T _v;

    inline constexpr static T plus_inf_value = std::numeric_limits<T>::max();       //!< 正の無限大
    inline constexpr static T minus_inf_value = std::numeric_limits<T>::min() + 1;  //!< 負の無限大
    inline constexpr static T nan_value = std::numeric_limits<T>::min();            //!< Not a Number
    static std::string str_plusinf;
    static std::string str_minusinf;
    static std::string str_nan;

   public:
    constexpr infint() noexcept : _v(0) {}
    constexpr infint(const T& t) noexcept : _v(t) {}
    constexpr infint(const infint& o) noexcept : _v(o._v) {}
    infint(std::string_view sv) noexcept(false) {
        std::string s(sv);
        std::for_each(s.begin(), s.end(), [](char& c) {
            if ('A' <= c && c <= 'Z') {
                c = c - 'A' + 'a';
            }
        });
        if (s == "plusinf") {
            _v = plus_inf_value;
        } else if (s == "inf") {
            _v = plus_inf_value;
        } else if (s == "minusinf") {
            _v = minus_inf_value;
        } else if (s == "nan") {
            _v = nan_value;
        } else {
            _v = std::stoll(s);
        }
    }

    /**
     * 内部表現をそのまま返す
     */
    constexpr T val() const noexcept { return _v; }

    constexpr bool is_nan() const noexcept { return _v == nan_value; }
    constexpr bool is_plus_inf() const noexcept { return _v == plus_inf_value; }
    constexpr bool is_minus_inf() const noexcept { return _v == minus_inf_value; }
    constexpr bool is_inf() const noexcept { return _v == plus_inf_value; }
    constexpr bool is_finite() const noexcept { return minus_inf_value < _v && _v < plus_inf_value; }

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
        if (_v == nan_value) return _v;
        return -_v;
    }
    constexpr infint& operator*=(const infint& o) noexcept {
        if (is_nan() || o.is_nan()) {
            _v = nan_value;
            return *this;
        }
        if (!is_finite() || !o.is_finite()) {
            if (_v == 0 || o._v == 0) {
                _v = nan_value;
            } else if ((_v > 0) ^ (o._v > 0)) {
                _v = minus_inf_value;
            } else {
                _v = plus_inf_value;
            }
            return *this;
        }
        if (_v == 0 || o._v == 0) {
            _v = 0;
            return *this;
        }
        // 有限の値の最大は plus_inf_value - 1
        if (std::abs(_v) > (plus_inf_value - 1) / std::abs(o._v)) {
            if ((_v > 0) ^ (o._v > 0)) {
                _v = minus_inf_value;
            } else {
                _v = plus_inf_value;
            }
            return *this;
        }
        _v *= o._v;
        return *this;
    }
    constexpr infint& operator/=(const infint& o) noexcept {
        if (is_nan() || o.is_nan()) {
            _v = nan_value;
            return *this;
        }
        if (!is_finite() && !o.is_finite()) {
            _v = nan_value;
            return *this;
        }
        if (o._v == 0) {
            _v = nan_value;
            return *this;
        }
        if (!o.is_finite()) {
            _v = 0;
            return *this;
        }
        if (!is_finite()) {
            // ±inf / 有限 = ±inf (符号は商の符号)
            if ((_v > 0) ^ (o._v > 0)) {
                _v = minus_inf_value;
            } else {
                _v = plus_inf_value;
            }
            return *this;
        }
        _v /= o._v;
        return *this;
    }
    constexpr infint& operator%=(const infint& o) noexcept {
        if (is_nan() || o.is_nan()) {
            _v = nan_value;
            return *this;
        }
        if (!is_finite() && !o.is_finite()) {
            _v = nan_value;
            return *this;
        }
        if (o._v == 0) {
            _v = nan_value;
            return *this;
        }
        if (!is_finite()) {
            // ±inf % 有限 は定まらない (浮動小数点数の fmod と同じく NaN)
            _v = nan_value;
            return *this;
        }
        if (!o.is_finite()) {
            return *this;
        }
        _v %= o._v;
        return *this;
    }
    constexpr infint& operator+=(const infint& o) noexcept {
        if (is_nan() || o.is_nan()) {
            _v = nan_value;
            return *this;
        }
        if (!is_finite() || !o.is_finite()) {
            if (!is_finite() && !o.is_finite()) {
                if (is_plus_inf() ^ o.is_plus_inf()) {
                    _v = nan_value;
                }
            } else if (!o.is_finite()) {
                _v = o._v;
            }
            return *this;
        }
        if (o._v > 0) {
            if (_v >= plus_inf_value - o._v) {
                _v = plus_inf_value;
                return *this;
            }
        } else if (o._v < 0) {
            if (_v <= minus_inf_value - o._v) {
                _v = minus_inf_value;
                return *this;
            }
        }
        _v += o._v;
        return *this;
    }
    constexpr infint& operator-=(const infint& o) noexcept {
        if (is_nan() || o.is_nan()) {
            _v = nan_value;
            return *this;
        }
        if (!is_finite() || !o.is_finite()) {
            if (!is_finite() && !o.is_finite()) {
                if (is_plus_inf() == o.is_plus_inf()) {
                    _v = nan_value;
                }
            } else if (!o.is_finite()) {
                _v = -o._v;
            }
            return *this;
        }
        if (o._v > 0) {
            if (_v <= minus_inf_value + o._v) {
                _v = minus_inf_value;
                return *this;
            }
        } else if (o._v < 0) {
            if (_v >= plus_inf_value + o._v) {
                _v = plus_inf_value;
                return *this;
            }
        }
        _v -= o._v;
        return *this;
    }
    static std::string type_str() { return "infint<>"; }
    std::string initializer_str() const {
        if (_v == plus_inf_value) {
            return "{\"plusinf\"}";
        } else if (_v == minus_inf_value) {
            return "{\"minusinf\"}";
        } else if (_v == nan_value) {
            return "{\"nan\"}";
        } else {
            return "{" + std::to_string(_v) + "}";
        }
    }
    std::string repr() const {
        return this->type_str() + this->initializer_str();
    }
    std::string to_string() const {
        if (_v == plus_inf_value) {
            return str_plusinf;
        } else if (_v == minus_inf_value) {
            return str_minusinf;
        } else if (_v == nan_value) {
            return str_nan;
        } else {
            return std::to_string(_v);
        }
    }
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const infint& rhs) {
        os << rhs.to_string();
        return os;
    }
    static void set_str_plusinf(std::string s) {
        str_plusinf = s;
    }
    static void set_str_minusinf(std::string s) {
        str_minusinf = s;
    }
    static void set_str_nan(std::string s) {
        str_nan = s;
    }

    /**
     * 正の無限大
     */
    constexpr static infint plus_inf() noexcept {
        return plus_inf_value;
    }
    /**
     * 負の無限大
     */
    constexpr static infint minus_inf() noexcept {
        return minus_inf_value;
    }
    /**
     * Not a Number
     */
    constexpr static infint nan() noexcept {
        return nan_value;
    }
    /**
     * 正の無限大(plus_inf と同じ)
     */
    constexpr static infint inf() noexcept {
        return plus_inf_value;
    }
    /**
     * 正の無限大でない最大値
     */
    constexpr static infint max() noexcept {
        return plus_inf_value - 1;
    }
    /**
     * 負の無限大でない最小値
     */
    constexpr static infint min() noexcept {
        return minus_inf_value + 1;
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
        if (t1.is_nan() || t2.is_nan()) return false;
        return t1._v < t2._v;
    }
    friend constexpr bool operator>(const infint& t1, const infint& t2) noexcept {
        if (t1.is_nan() || t2.is_nan()) return false;
        return t1._v > t2._v;
    }
    friend constexpr bool operator<=(const infint& t1, const infint& t2) noexcept {
        if (t1.is_nan() || t2.is_nan()) return false;
        return t1._v <= t2._v;
    }
    friend constexpr bool operator>=(const infint& t1, const infint& t2) noexcept {
        if (t1.is_nan() || t2.is_nan()) return false;
        return t1._v >= t2._v;
    }
    friend constexpr bool operator==(const infint& t1, const infint& t2) noexcept {
        return t1._v == t2._v;
    }
    friend constexpr bool operator!=(const infint& t1, const infint& t2) noexcept {
        return t1._v != t2._v;
    }
};
template <typename T>
std::string infint<T>::str_plusinf = "inf";
template <typename T>
std::string infint<T>::str_minusinf = "-inf";
template <typename T>
std::string infint<T>::str_nan = "nan";
