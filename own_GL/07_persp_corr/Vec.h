#ifndef VEC_H
#define VEC_H

#include <fstream>
#include <array>
#include <algorithm>

/*
 * Class Vec:
 *      using fixed size array structure from std (std::array<Num, N>)
 *      Num type hints that the type should be numeric
 *      Decided to use fixed size array because the Vec class is going to be
 *      used mainly for short (2 - 4 elements) arrays
 */
template <size_t N, class Num>
class Vec {
    std::array<Num, N> A_;
public:
    // ctors
    Vec(const Num& = 0); // fill array with one value
    constexpr Vec(const std::array<Num, N>&); // init from std::array
    Vec(const std::initializer_list<Num>&); // from initializer_list
    template <size_t M, class Comp>
    Vec(const Vec<M, Comp>&); // templated copy ctor: e.g., double to int
    //Vec(Vec&&) = default; // some problems with move ctor
    template <size_t M, class Comp> // templated assignment operator
    Vec& operator=(const Vec<M, Comp>&);
    ~Vec() = default;

    // templated compound arithemtic operators
    template <class Comp>
    Vec& operator+=(const Comp&);
    template <class Comp>
    Vec& operator+=(const Vec<N, Comp>&);
    template <class Comp>
    Vec& operator-=(const Comp&);
    template <class Comp>
    Vec& operator-=(const Vec<N, Comp>&);
    template <class Comp>
    Vec& operator*=(const Comp&);
    template <class Comp>
    Vec& operator/=(const Comp&);

    // iterators: using in loops (for (auto a: v))
    using iterator = typename std::array<Num, N>::iterator;
    using const_iterator = typename std::array<Num, N>::const_iterator;
    iterator begin() { return A_.begin(); }
    const_iterator begin() const { return A_.begin(); }
    const_iterator cbegin() const { return A_.cbegin(); }
    iterator end() { return A_.end(); }
    const_iterator end() const { return A_.end(); }
    const_iterator cend() const { return A_.cend(); }

    // index and range-check index operators
    Num& operator[](const size_t i) { return A_[i]; }
    const Num& operator[](const size_t i) const { return A_[i]; }
    Num& at(const size_t i) { return A_.at(i); }
    const Num& at(const size_t i) const { return A_.at(i); }

    Num& x() { return A_[0]; }
    const Num& x() const { return A_[0]; }
    Num& y() { return A_[1]; }
    const Num& y() const { return A_[1]; }
    Num& z() { return A_[2]; }
    const Num& z() const { return A_[2]; }

    // default value for w is 1, so assuming we use Vec for points
    //Num w() { return N > 3 ? A_[3] : N > 2 ? A_[2]; }
    //const Num w() const {  return N > 3 ? A_[3] : N > 2 ? A_[2] : 1; }

    //Num& x() { return N > 0 ? A_[0] : 0; }
    //const Num& x() const { return N > 0 ? A_[0] : 0; }
    //Num& y() { return N > 1 ? A_[1] : 0; }
    //const Num& y() const { return N > 1 ? A_[1] : 0; }
    //Num& z() { return N > 2 ? A_[2] : 0; }
    //const Num& z() const { return N > 2 ? A_[2] : 0; }
    //// default value for w is 1, so assuming we use Vec for points
    //Num w() { return N > 3 ? A_[3] : N > 2 ? A_[2] : 1; }
    //const Num w() const {  return N > 3 ? A_[3] : N > 2 ? A_[2] : 1; }

    constexpr size_t size() const { return N; }
    constexpr std::array<Num, N>& values() const { return A_; }

    // methods concerning normalization
    double norm() const { return std::sqrt((*this) * (*this)); }
    //Vec& normalize() { *this /= norm(); return *this; }
    Vec& normalize() { return *this /= norm(); }
    const Vec normalize() const { return *this / norm(); }
    //double norm() const;
    //Vec normalize();
    //const Vec normalize() const;
};

/*
 * ------------------ Vec ctors and operators ------------------
 */
template <size_t N, class Num>
Vec<N, Num>::Vec(const Num& val): A_() { A_.fill(val); }

template <size_t N, class Num>
constexpr Vec<N, Num>::Vec(const std::array<Num, N> &A): A_(A) { }

template <size_t N, class Num>
Vec<N, Num>::Vec(const std::initializer_list<Num> &IL): A_() {
    auto iter = std::begin(IL);
    for (size_t i {0}; i < std::min(N, IL.size()); ++i, ++iter) A_[i] = *iter;
}

template <size_t N, class Num> template <size_t M, class Comp>
Vec<N, Num>::Vec(const Vec<M, Comp> &o): A_() {
    for (auto i = std::min(N, M); i--; A_[i] = o[i]) { }
}

template <size_t N, class Num> template <size_t M, class Comp>
Vec<N, Num>& Vec<N, Num>::operator=(const Vec<M, Comp> &o) {
    for (auto i = std::min(N, M); i--; A_[i] = o[i]) { }
    return *this;
}

template <size_t N, class Num>
std::istream& operator>>(std::istream &is, Vec<N, Num> &v) {
    for (auto &a: v)
        if (!(is >> a)) break;
    return is;
}

template <size_t N, class Num>
std::ostream& operator<<(std::ostream &os, const Vec<N, Num> &v) {
    os << "{ ";
    for (const auto &a: v) os << +a << ' ';
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

/*
 * ------------------ Vec arithmetic compound operators ------------------
 */
// sum assignment: add a value to a vector
template <size_t N, class Num> template <class Comp>
inline Vec<N, Num>& Vec<N, Num>::operator+=(const Comp &rhs) {
    for (auto &a: A_) a += rhs;
    return *this;
}

// sum assignment: add a vector to a vector
template <size_t N, class Num> template <class Comp>
inline Vec<N, Num>& Vec<N, Num>::operator+=(const Vec<N, Comp> &rhs) {
    for (auto i = N; i--; A_[i] += rhs[i]) { }
    return *this;
}

// difference assignment: subtract a value from a vector
template <size_t N, class Num> template <class Comp>
inline Vec<N, Num>& Vec<N, Num>::operator-=(const Comp &rhs) {
    for (auto &a: A_) a -= rhs;
    return *this;
}

// difference assignment: subtract a vector from a vector
template <size_t N, class Num> template <class Comp>
inline Vec<N, Num>& Vec<N, Num>::operator-=(const Vec<N, Comp> &rhs) {
    for (auto i = N; i--; A_[i] -= rhs[i]) { }
    return *this;
}

// multiply a vector by a value
template <size_t N, class Num> template <class Comp>
inline Vec<N, Num>& Vec<N, Num>::operator*=(const Comp &rhs) {
    for (auto &a: A_) a *= rhs;
    return *this;
}

// divide a vector by a value
template <size_t N, class Num> template <class Comp>
inline Vec<N, Num>& Vec<N, Num>::operator/=(const Comp &rhs) {
    for (auto &a: A_) a /= rhs;
    return *this;
}

/*
 * ------------------ Vec arithmetic operators ------------------
 */
// sum of two vectors
template <size_t N, class Num, class Comp>
inline Vec<N, class std::common_type<Num, Comp>::type> operator+(
        const Vec<N, Num> &lhs, const Vec<N, Comp> &rhs) {
    Vec<N, class std::common_type<Num, Comp>::type> res {lhs};
    return res += rhs;
}

// add a value to a vector
template <size_t N, class Num, class Comp>
inline Vec<N, class std::common_type<Num, Comp>::type> operator+(
        const Vec<N, Num> &lhs, const Comp &rhs) {
    Vec<N, class std::common_type<Num, Comp>::type> res {lhs};
    return res += rhs;
}

// difference operation
template <size_t N, class Num, class Comp>
inline Vec<N, class std::common_type<Num, Comp>::type> operator-(
        const Vec<N, Num> &lhs, const Vec<N, Comp> &rhs) {
    Vec<N, class std::common_type<Num, Comp>::type> res {lhs};
    return res -= rhs;
}

// subtract a value from a vector
template <size_t N, class Num, class Comp>
inline Vec<N, class std::common_type<Num, Comp>::type> operator-(
        const Vec<N, Num> &lhs, const Comp &rhs) {
    Vec<N, class std::common_type<Num, Comp>::type> res {lhs};
    return res -= rhs;
}

// multiply a vector by a value
template <size_t N, class Num, class Comp>
inline Vec<N, class std::common_type<Num, Comp>::type> operator*(
        const Vec<N, Num> &lhs, const Comp &rhs) {
    Vec<N, class std::common_type<Num, Comp>::type> res {lhs};
    return res *= rhs;
}

// divide a vector by a value
template <size_t N, class Num, class Comp>
inline Vec<N, class std::common_type<Num, Comp>::type> operator/(
        const Vec<N, Num> &lhs, const Comp &rhs) {
    Vec<N, class std::common_type<Num, Comp>::type> res {lhs};
    return res /= rhs;
}

// dot product
template <size_t N, class Num, class Comp>
inline typename std::common_type<Num, Comp>::type operator*(
        const Vec<N, Num> &lhs, const Vec<N, Comp> &rhs) {
    //return std::inner_product(std::begin(lhs), std::end(lhs), std::begin(rhs),
    //        0.0, std::plus<double>(), std::multiplies<double>());
    typename std::common_type<Num, Comp>::type res = 0;
    for (auto i = N; i--; res += lhs[i] * rhs[i]) { }
    return res;
}

template <size_t N, class Num, class Comp>
inline typename std::common_type<Num, Comp>::type dot(
        const Vec<N, Num> &lhs, const Vec<N, Comp> &rhs) {
    //return std::inner_product(std::begin(lhs), std::end(lhs), std::begin(rhs),
    //        0.0, std::plus<double>(), std::multiplies<double>());
    typename std::common_type<Num, Comp>::type res = 0;
    for (auto i = N; i--; res += lhs[i] * rhs[i]) { }
    return res;
}

// cross product: we have it implemented only for 3-element vector
template <class Num, class Comp>
constexpr Vec<3, class std::common_type<Num, Comp>::type> operator^(
        const Vec<3, Num> &lhs, const Vec<3, Comp> &rhs) {
    return std::array<class std::common_type<Num, Comp>::type, 3> {
        lhs[1] * rhs[2] - lhs[2] * rhs[1], // y * z - z * y
        lhs[2] * rhs[0] - lhs[0] * rhs[2], // z * x - x * z
        lhs[0] * rhs[1] - lhs[1] * rhs[0], // x * y - y * x
    };
}

template <class Num, class Comp>
constexpr Vec<3, class std::common_type<Num, Comp>::type> cross(
        const Vec<3, Num> &lhs, const Vec<3, Comp> &rhs) {
    return std::array<class std::common_type<Num, Comp>::type, 3> {
        lhs[1] * rhs[2] - lhs[2] * rhs[1], // y * z - z * y
        lhs[2] * rhs[0] - lhs[0] * rhs[2], // z * x - x * z
        lhs[0] * rhs[1] - lhs[1] * rhs[0], // x * y - y * x
    };
}

/*
 * ------------------ Normalization stuff ------------------
 */
template <size_t N, class Num>
void normalize(Vec<N, Num> &v) {
    v /= v.norm();
}

/*
 * change the vector's size:
 * in case if the new size is bigger, we can set the filling value to fill the
 * trailing part of the vector
 */
template <size_t M, size_t N, class Num>
Vec<M, Num> resize(const Vec<N, Num> &v, const Num& fill = 1) {
    Vec<M, Num> res;
    for (size_t i = N; i < M; ++i)
        res[i] = fill;
    for (size_t i = N; i--; res[i] = v[i]) { }
    return res;
}

#endif

