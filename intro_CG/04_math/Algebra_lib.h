#ifndef ALGEBRA_LIB_H
#define ALGEBRA_LIB_H

namespace Algebra_lib {

// set the maximum length for a vector
static constexpr int max_vec_size {4};

// declare the Vec class (Vector)
template <int N, class Num> class Vec;

// declare the Mat class (Matrix)
template <int R, int C, class Num> class Mat;

} /* Algebra_lib namespace */

#endif /* ALGEBRA_LIB_H */

