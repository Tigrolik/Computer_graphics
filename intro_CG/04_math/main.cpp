/*
 * Working with matrices: transformations
 */

//#include "PPM_lib.h"
//#include "Geometry.h"
#include "Vec.h"
#include "Mat.h"
#include <iostream>

void test_vec() {
    using namespace Algebra_lib;

    Vec<3, int> v1(5);
    std::cout << v1 << '\n';
    //Vec<0, int> v_false1 {5};
    //Vec<8, int> v_false2 {5};
    constexpr Vec<4, int> v2 {std::array<int, 4> {1, 2, 3, 4}};
    std::cout << v2 << '\n';
    Vec<4, double> v3 {1.5, 2, 3.4, 4.1};
    std::cout << v3 << '\n';

    Vec<3, int> v4 {v1};
    std::cout << v4 << '\n';
}

void test_mat() {
    using namespace Algebra_lib;
    //template Mat<2, 4, int>;

    Mat<2, 4, int> mi1 (7);
    std::cout << mi1 << '\n';

    Mat<4, 2, double> md1 (Vec<2, double> {2.8, 5.7});
    std::cout << md1 << '\n';
}

int main() {

    //test_vec();
    test_mat();

    return 0;
}

