#ifndef VEC_H
#define VEC_H

#include "Algebra_lib.h"
#include <array>
#include <iostream>
#include <fstream>

namespace Algebra_lib {

//template <size_t R, size_t C, class Num>
//    Vec<R, Num> operator*(const Mat<R, C, Num>&, const Vec<C, Num>&);

template <int N, class Num>
class Vec {
    static_assert(N > 0, "N must be positive");
    static_assert(N <= max_vec_size, "N must not exceed max_vec_size");
public:
    explicit Vec(const Num = Num{});
    explicit constexpr Vec(const std::array<Num, N>&);
    explicit Vec(const std::initializer_list<Num>&);

    // copy ctors
    Vec(const Vec&) = default;
    template <class Comp>
        explicit Vec(const Vec<N, Comp>&);

    Vec& operator=(const Vec&) = default;
    template <class Comp>
        Vec& operator=(const Vec<N, Comp>&);

    Vec(Vec&&) = default;
    Vec& operator=(Vec&&) = default;
    ~Vec() = default;

    using value_type = Num;
    using iterator = typename std::array<Num, N>::iterator;
    using const_iterator = typename std::array<Num, N>::const_iterator;
    // iterators: using in loops (for (auto a: v))
    iterator begin() { return V_.begin(); }
    const_iterator begin() const { return V_.begin(); }
    const_iterator cbegin() const { return V_.cbegin(); }
    iterator end() { return V_.end(); }
    const_iterator end() const { return V_.end(); }
    const_iterator cend() const { return V_.cend(); }

    Num& operator[](const int i) { return V_[i]; }
    const Num& operator[](const int i) const { return V_[i]; }

    //template <size_t R, size_t C, class Comp>
    //    Vec<R, Comp> operator*<>(const Mat<R, C, Comp>&, const Vec<C, Comp>&);
    //template <size_t R>
    //    friend Vec<R, Num> operator*<>(const Mat<R, N, Num>&,
    //            const Vec<N, Num>&);

private:
    std::array<Num, N> V_;
};

// default constructor with a possibility to set the default value
template <int N, class Num>
Vec<N, Num>::Vec(const Num val): V_{} {
    V_.fill(val);
}

// constructor using std::array
template <int N, class Num>
constexpr Vec<N, Num>::Vec(const std::array<Num, N> &A): V_{A} { }

// constructor using initializer_list
template <int N, class Num>
Vec<N, Num>::Vec(const std::initializer_list<Num> &IL): V_{} {
    auto iter = std::begin(IL);
    for (auto i = 0; i < std::min<int>(N, IL.size()); ++i, ++iter)
        V_[i] = *iter;
}


// output operator
template <int N, class Num>
std::ostream& operator<<(std::ostream &os, const Vec<N, Num>& V) {
    os << "{ ";
    for (const auto v: V)
        os << +v << ' ';
    return os << '}';
}

} // Algebra_lib

#endif /* VEC_H */

