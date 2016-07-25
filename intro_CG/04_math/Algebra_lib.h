#ifndef ALGEBRA_LIB_H
#define ALGEBRA_LIB_H

#include <cstdlib> // for size_t

namespace Algebra_lib {

// set the maximum length for a vector
static constexpr int max_vec_size {9};

// declare the Vec class (Vector)
template <size_t N, class Num> class Vec;

// declare the Mat class (Matrix)
template <size_t R, size_t C, class Num> class Mat;

}

#endif /* ALGEBRA_LIB_H */

