#ifndef VEC_H
#define VEC_H

#include "Algebra_lib.h"
#include <array>
#include <fstream>
#include <cmath> // std::sqrt function for norm()

namespace Algebra_lib {

template <size_t N, class Num>
class Vec {
    static_assert(N > 0, "Vec size must be positive");
    static_assert(N <= max_vec_size, "Vec size must not exceed max_vec_size");
public:
    /*
     * Ctors, assignment operators and dtor
     */
    // ctors
    constexpr Vec();
    explicit constexpr Vec(const std::array<Num, N>&);
    explicit Vec(const std::initializer_list<Num>&);

    // copy ctors
    constexpr Vec(const Vec&) = default;
    //constexpr Vec(const Vec&);
    //template <class Comp>
    //    Vec(const Vec<N, Comp>&);

    // assignment operators
    Vec& operator=(const Vec&) = default;
    //template <class Comp>
    //    Vec& operator=(const Vec<N, Comp>&);

    // move ctor and assignment
    Vec(Vec&&) = default;
    Vec& operator=(Vec&&) = default;

    // dtor
    ~Vec() = default;
    /* --- end ctors, assignment operators and dtor --- */

    // cast operators
    explicit operator Vec<N, double>() const;
    //template <class Comp>
    //    explicit operator Vec<N, Comp>() const;

    /*
     * Compound arithemtic operators
     */
    Vec& operator+=(const Num&);
    Vec& operator+=(const Vec&);
    Vec& operator-=(const Num&);
    Vec& operator-=(const Vec&);
    Vec& operator*=(const Num&);
    Vec& operator*=(const Vec&);
    Vec& operator/=(const Num&);
    Vec& operator/=(const Vec&);
    /* --- end compound arithemtic operators --- */

    using value_type     = Num;
    using size_type      = typename std::array<Num, N>::size_type;
    using iterator       = typename std::array<Num, N>::iterator;
    using const_iterator = typename std::array<Num, N>::const_iterator;

    // iterators: using in loops (for (auto a: v))
    iterator begin() { return V_.begin(); }
    const_iterator begin() const { return V_.begin(); }
    const_iterator cbegin() const { return V_.cbegin(); }
    iterator end() { return V_.end(); }
    const_iterator end() const { return V_.end(); }
    const_iterator cend() const { return V_.cend(); }

    value_type& operator[](const size_t i) { return V_[i]; }
    const value_type& operator[](const size_t i) const { return V_[i]; }
    value_type& at(const size_t i) { return V_.at(i); }
    const value_type& at(const size_t i) const { return V_.at(i); }

    Num& x() { return V_[0]; }
    const Num& x() const { return V_[0]; }
    Num& y() { return at(1); }
    const Num& y() const { return at(1); }
    Num& z() { return at(2); }
    const Num& z() const { return at(2); }
    Num& w() { return N > 3 ? at(3) : at(2); }
    const Num& w() const { return N > 3 ? at(3) : at(2); }

    constexpr size_t size() const { return N; }
    constexpr std::array<Num, N> values() const { return V_; }

    // fill a vector with single value
    void fill(const Num&);

    double norm() const { return std::sqrt(dot(*this, *this)); }
    //Vec<N, double> normalize() { return (Vec<N, double>(*this)) /= norm(); }
    Vec<N, Num> normalize() { return (*this) /= Num(norm()); }
    //const Vec<N, double> normalize() const { return (Vec<N, double>(*this)) /
    //    norm(); }
    const Vec<N, Num> normalize() const { return (*this) / Num(norm()); }

private:
    std::array<Num, N> V_;
};

/*
 * Ctors and assignment operators
 */
// default constructor
template <size_t N, class Num>
constexpr Vec<N, Num>::Vec(): V_{} { }

// constructor using std::array
template <size_t N, class Num>
constexpr Vec<N, Num>::Vec(const std::array<Num, N> &A): V_{A} { }

// constructor using initializer_list: the Vec is initialized to zero values; if
// the list is too short, then the rest of the Vec is zero; if the list is too
// long, then the excessive values are ignored
template <size_t N, class Num>
inline Vec<N, Num>::Vec(const std::initializer_list<Num> &IL): V_{} {
    auto iter = std::begin(IL);
    for (size_t i {0}; i < std::min(N, IL.size()); ++i, ++iter)
        V_[i] = *iter;
}

//template <size_t N, class Num>
//constexpr Vec<N, Num>::Vec(const Vec<N, Num> &V): Vec<N, Num>{V.values()} { }

// copy constructor with conversion from Comp to Num
//template <size_t N, class Num> template <class Comp>
//inline Vec<N, Num>::Vec(const Vec<N, Comp> &o): V_{} {
//    for (auto i = N; i--; V_[i] = static_cast<Num>(o[i])) { }
//}

// copy assignment with conversion from Comp to Num
//template <size_t N, class Num> template <class Comp>
//inline Vec<N, Num>& Vec<N, Num>::operator=(const Vec<N, Comp> &o) {
//    if (*this != &o)
//        for (auto i = N; i--; V_[i] = static_cast<Num>(o[i])) { }
//    return *this;
//}
/* --- end ctors and assignment operators --- */

// cast operators
// convert a vector into a double vector
template <size_t N, class Num>
inline Vec<N, Num>::operator Vec<N, double>() const {
    Vec<N, double> res {};
    for (size_t i {0}; i < N; ++i)
        res[i] = double(V_[i]);
    return res;
}

//template <size_t N, class Num> template <class Comp>
//Vec<N, Num>::operator Vec<N, Comp>() const {
//    Vec<N, Comp> res {};
//    for (size_t i {0}; i < N; ++i)
//        res[i] = static_cast<Comp>(V_[i]);
//    return res;
//}

/*
 * ------------------ Vec arithmetic compound operators ------------------
 */
// sum assignment: add a value to a vector
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator+=(const Num &rhs) {
    for (auto &a: V_)
        a += rhs;
    return *this;
}

// sum assignment: add a vector to a vector
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator+=(const Vec<N, Num> &rhs) {
    for (auto i = N; i--; V_[i] += rhs[i]) { }
    return *this;
}

// difference assignment: subtract a value from a vector
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator-=(const Num &rhs) {
    for (auto &a: V_)
        a -= rhs;
    return *this;
}

// difference assignment: subtract a vector from a vector
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator-=(const Vec<N, Num> &rhs) {
    for (auto i = N; i--; V_[i] -= rhs[i]) { }
    return *this;
}

// multiply a vector by a value
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator*=(const Num &rhs) {
    for (auto &a: V_)
        a *= rhs;
    return *this;
}

// multiply a vector by another vector element-wize
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator*=(const Vec<N, Num> &rhs) {
    for (auto i = N; i--; V_[i] *= rhs[i]) { }
    return *this;
}

// divide a vector by a value
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator/=(const Num &rhs) {
    for (auto &a: V_)
        a /= rhs;
    return *this;
}

// divide a vector by another vector element-wize
template <size_t N, class Num>
inline Vec<N, Num>& Vec<N, Num>::operator/=(const Vec<N, Num> &rhs) {
    for (auto i = N; i--; V_[i] /= rhs[i]) { }
    return *this;
}
/* --------------- end Vec arithmetic compound operators --------------- */

/*
 * ------------------ Vec arithmetic operators ------------------
 */
// add a value to a vector
template <size_t N, class Num>
inline Vec<N, Num> operator+(const Vec<N, Num> &lhs, const Num &rhs) {
    Vec<N, Num> res {lhs};
    return res += rhs;
}

// sum of two vectors
template <size_t N, class Num>
inline Vec<N, Num> operator+(const Vec<N, Num> &lhs, const Vec<N, Num> &rhs) {
    Vec<N, Num> res {lhs};
    return res += rhs;
}

// subtract a value to a vector
template <size_t N, class Num>
inline Vec<N, Num> operator-(const Vec<N, Num> &lhs, const Num &rhs) {
    Vec<N, Num> res {lhs};
    return res -= rhs;
}

// difference of two vectors
template <size_t N, class Num>
inline Vec<N, Num> operator-(const Vec<N, Num> &lhs, const Vec<N, Num> &rhs) {
    Vec<N, Num> res {lhs};
    return res -= rhs;
}

// multiply a vector by a value
template <size_t N, class Num>
inline Vec<N, Num> operator*(const Vec<N, Num> &lhs, const Num &rhs) {
    Vec<N, Num> res {lhs};
    return res *= rhs;
}

// product of two vectors element-wise
template <size_t N, class Num>
inline Vec<N, Num> operator*(const Vec<N, Num> &lhs, const Vec<N, Num> &rhs) {
    Vec<N, Num> res {lhs};
    return res *= rhs;
}

// divide a vector by a value
template <size_t N, class Num>
inline Vec<N, Num> operator/(const Vec<N, Num> &lhs, const Num &rhs) {
    Vec<N, Num> res {lhs};
    return res /= rhs;
}

// fraction of two vectors element-wise
template <size_t N, class Num>
inline Vec<N, Num> operator/(const Vec<N, Num> &lhs, const Vec<N, Num> &rhs) {
    Vec<N, Num> res {lhs};
    return res /= rhs;
}

// dot product
template <size_t N, class Num>
inline Num dot(const Vec<N, Num> &lhs, const Vec<N, Num> &rhs) {
    //return std::inner_product(std::begin(lhs), std::end(lhs), std::begin(rhs),
    //        0.0, std::plus<double>(), std::multiplies<double>());
    Num res = {};
    for (auto i = N; i--; res += lhs[i] * rhs[i]) { }
    return res;
}

// cross product: implemented only for 3-element vector
template <class Num>
constexpr Vec<3, Num> cross(const Vec<3, Num> &lhs, const Vec<3, Num> &rhs) {
    return Vec<3, Num> {
        lhs[1] * rhs[2] - lhs[2] * rhs[1], // y * z - z * y
        lhs[2] * rhs[0] - lhs[0] * rhs[2], // z * x - x * z
        lhs[0] * rhs[1] - lhs[1] * rhs[0], // x * y - y * x
    };
}
/* ------------------ end Vec arithmetic operators ------------------*/

/*
 * Input-Output, comparison
 */
// input operator
template <size_t N, class Num>
std::istream& operator>>(std::istream &is, Vec<N, Num> &V) {
    for (auto &a: V)
        if (!(is >> a)) break;
    return is;
}

// output operator
template <size_t N, class Num>
inline std::ostream& operator<<(std::ostream &os, const Vec<N, Num>& V) {
    os << "{ ";
    for (const auto v: V)
        os << +v << ' ';
    return os << '}';
}

/*
 * using very low value (1E-14) to compare values instead of zero due to
 * precision issues
*/
template <size_t N, class Num, size_t M, class Comp>
inline bool operator==(const Vec<N, Num> &v1, const Vec<M, Comp> &v2) {
    if (M != N) return false;
    for (size_t i {0}; i < N; ++i)
        if (std::abs(v1[i] - v2[i]) > 1E-14) return false;
        //if (v1[i] != v2[i]) return false;
    return true;
}

template <size_t N, class Num, size_t M, class Comp>
inline bool operator!=(const Vec<N, Num> &v1, const Vec<M, Comp> &v2) {
    return !(v1 == v2);
}
/* --- end Input-Output, comparison --- */

/*
 * Additional methods and functions
 */
// fill Vec with a value
template <size_t N, class Num>
inline void Vec<N, Num>::fill(const Num& val) {
    //std::fill(std::begin(V_), std::end(V_), val);
    for (auto &a: V_)
        a = val;
}

/*
 * change the vector's size:
 * in case if the new size is bigger, the trailing part is filled with default
 * values we can set the filling value to fill the
 * trailing part of the vector
 */
template <size_t M, size_t N, class Num>
Vec<M, Num> resize(const Vec<N, Num> &v) {
    Vec<M, Num> res;
    for (size_t i {0}; i < std::min(M, N); ++i)
        res[i] = v[i];
    return res;
}

// if the new size is bigger, the trailing part is filled with given value
template <size_t M, size_t N, class Num>
Vec<M, Num> resize(const Vec<N, Num> &v, const Num& fill) {
    Vec<M, Num> res = resize<M>(v);
    for (size_t i = N; i < M; ++i)
        res[i] = fill;
    return res;
}
/* --- end additional methods and functions --- */

} // end Algebra_lib

#endif /* VEC_H */

