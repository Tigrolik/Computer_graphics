/*
 * Working with matrices: transformations
 */

//#include "PPM_lib.h"
//#include "Geometry.h"
#include "Vec.h"
#include "Mat.h"
#include "PPM_lib.h"
#include "Geometry.h"
#include <iostream>

// manual instantiation: might be useful to reduce compliation time
template class Algebra_lib::Mat<3, 3, int>;
template class Algebra_lib::Mat<3, 3, double>;

// shorthand names for common types
using Vec3i = Algebra_lib::Vec<3, int>;
using Vec3d = Algebra_lib::Vec<3, double>;
using Mat3i = Algebra_lib::Mat<3, 3, int>;
using Mat3d = Algebra_lib::Mat<3, 3, double>;

// helper function: Point to Vec3i
Vec3i point_to_vec(const PPM_lib::Point& p) {
    return Vec3i {p.x(), p.y(), 1};
}

// helper function: Vec3i to Point
PPM_lib::Point vec_to_point(const Vec3i& v) {
    return PPM_lib::Point {v.x(), v.y()};
}

// translation matrix
Mat3i translation(const int dx, const int dy) {
    auto M = Algebra_lib::eye<3>();
    M[2][0] = dx;
    M[2][1] = dy;
    return M;
}

// shear in x axis matrix
Mat3d shear_x(const double sh_x) {
    using namespace Algebra_lib;
    auto M = Mat3d(eye<3>());
    M[1][0] = sh_x;
    return M;
}

// scale matrix
Mat3d scale(const double sx, const double sy) {
    auto M = Mat3d {};
    M[0][0] = sx;
    M[1][1] = sy;
    M[2][2] = 1;
    return M;
}

// rotation matrix
Mat3d rotation(const double a) {
    auto M = Mat3d {};
    const auto ang = a * PPM_lib::pi / 180.0;
    M[0][0] = std::cos(ang);
    M[1][1] = std::cos(ang);
    M[0][1] = std::sin(ang);
    M[1][0] = -std::sin(ang);
    M[2][2] = 1;
    return M;
}

// transform a triangle
void transform_tri(PPM_lib::Triangle& T, const Mat3i& M) {
    T = {vec_to_point(point_to_vec(T.p1()) * M),
        vec_to_point(point_to_vec(T.p2()) * M),
        vec_to_point(point_to_vec(T.p3()) * M)};
}

// transform a triangle
void transform_tri(PPM_lib::Triangle& T, const Mat3d& M) {
    const auto v1 = Vec3d(point_to_vec(T.p1())) * M;
    PPM_lib::Point p1 {std::round(v1.x()), std::round(v1.y())};
    const auto v2 = Vec3d(point_to_vec(T.p2())) * M;
    PPM_lib::Point p2 {std::round(v2.x()), std::round(v2.y())};
    const auto v3 = Vec3d(point_to_vec(T.p3())) * M;
    PPM_lib::Point p3 {std::round(v3.x()), std::round(v3.y())};
    T = {p1, p2, p3};
}

void insert_image(PPM_lib::RGB_Image &I, const PPM_lib::RGB_Image &J) {
    const auto w = J.width(), h = J.height();
    //PPM_lib::Rectangle r {0, 0, w, h};
    for (auto i = 0; i < std::min<int>(I.width(), w); ++i)
        for (auto j = 0; j < std::min<int>(I.height(), h); ++j)
            I[i][j] = J[i][j];
}

PPM_lib::GS_Color interp_gray_color(const PPM_lib::GS_Image& I,
        const PPM_lib::Point p1, const PPM_lib::Point p2,
        const PPM_lib::Point p3, const PPM_lib::Point p4,
        const double x, const double y) {
    const int min_x = p1.x(), max_x = p2.x(), min_y = p1.y(), max_y = p3.y();
    const PPM_lib::RGB_Color::value_type c1 = I[p1.x()][p1.y()];
    const PPM_lib::RGB_Color::value_type c2 = I[p2.x()][p2.y()];
    const PPM_lib::RGB_Color::value_type c3 = I[p3.x()][p3.y()];
    const PPM_lib::RGB_Color::value_type c4 = I[p4.x()][p4.y()];
    const double val1 = (max_x - x) * c1 + (x - min_x) * c2;
    const double val2 = (max_x - x) * c4 + (x - min_x) * c3;
    const unsigned char val = (max_y - y) * val1 + (y - min_y) * val2;
    return PPM_lib::GS_Color{val};
}

PPM_lib::RGB_Color interp_rgb_color(const PPM_lib::RGB_Image& I,
        const PPM_lib::Point p1, const PPM_lib::Point p2,
        const PPM_lib::Point p3, const PPM_lib::Point p4,
        const double x, const double y) {
    const int min_x = p1.x(), max_x = p2.x(), min_y = p1.y(), max_y = p3.y();
    const PPM_lib::RGB_Color c1 = I.color(p1.x(), p1.y());
    const PPM_lib::RGB_Color c2 = I.color(p2.x(), p2.y());
    const PPM_lib::RGB_Color c3 = I.color(p3.x(), p3.y());
    const PPM_lib::RGB_Color c4 = I.color(p4.x(), p4.y());
    //std::cout << +c1.red() << '\n';

    const double val_r1 = (max_x - x) * c1.red() + (x - min_x) * c2.red();
    const double val_r2 = (max_x - x) * c4.red() + (x - min_x) * c3.red();
    const unsigned char val_r = (max_y - y) * val_r1 + (y - min_y) * val_r2;

    const double val_g1 = (max_x - x) * c1.green() + (x - min_x) * c2.green();
    const double val_g2 = (max_x - x) * c4.green() + (x - min_x) * c3.green();
    const unsigned char val_g = (max_y - y) * val_g1 + (y - min_y) * val_g2;

    const double val_b1 = (max_x - x) * c1.blue() + (x - min_x) * c2.blue();
    const double val_b2 = (max_x - x) * c4.blue() + (x - min_x) * c3.blue();
    const unsigned char val_b = (max_y - y) * val_b1 + (y - min_y) * val_b2;
    return PPM_lib::RGB_Color{val_r, val_g, val_b};
}

void fill_rect(const PPM_lib::Rectangle& R, PPM_lib::RGB_Image& img,
        const PPM_lib::RGB_Image& I) {
    const int w = I.width(), h = I.height();
    const int rw = R.width(), rh = R.height();
    std::cout << rw << ' ' << rh << '\n';
    const double xq = w / double(rw);
    const double yq = h / double(rh);
    using namespace PPM_lib;
    for (int i {0}; i < rw; ++i)
        for (int j {0}; j < rh; ++j) {
            const double iq = xq * i, jq = yq * j;
            const int i1 = int(iq), i2 = i1 + 1;
            const int j1 = int(jq), j2 = j1 + 1;
            if (i2 < w && j2 < h) {
                PPM_lib::RGB_Color c = interp_rgb_color(I, Point{i1, j1},
                        Point{i2, j1}, Point{i2, j2}, Point{i1, j2}, iq, jq);
                img.set_color(i, j, c);
            }
        }
}

// test matrix transformations
void test_transform() {
    using namespace PPM_lib;
    constexpr int w {800}, h {600};
    RGB_Image I {w, h};
    const Point p1 {10, 10}, p2 {10, 30}, p3 {45, 10};
    Triangle t1 {p1, p2, p3};
    t1.fill(I, Color_name::red); t1.draw(I, Color_name::red);

    // test translation
    //transform_tri(t1, translation(50, 100));
    t1.transform(Mat3d(translation(50, 100)));
    t1.fill(I, Color_name::red); t1.draw(I, Color_name::red);
    const auto t2 = t1;

    // test scale
    auto cur_x = t1.p1().x(), cur_y = t1.p1().y();
    t1.transform(Mat3d(translation(-cur_x, -cur_y)) * scale(2.5, 2) *
            Mat3d(translation(cur_x, cur_y)));
    t1.fill(I, Color_name::red); t1.draw(I, Color_name::red);

    t2.fill(I, Color_name::yellow);
    t2.draw(I, Color_name::yellow);

    //// test rotation
    cur_x = t1.p1().x(), cur_y = t1.p1().y();
    t1.transform(Mat3d(translation(-cur_x, -cur_y)) * rotation(45) *
            Mat3d(translation(cur_x, cur_y)));
    t1.fill(I, Color_name::red); t1.draw(I, Color_name::red);
    t1.fill(I, Color_name::red); t1.draw(I, Color_name::red);

    // test shear
    cur_x = t1.p1().x(), cur_y = t1.p1().y();
    t1.transform(Mat3d(translation(-cur_x, -cur_y)) * shear_x(1.5) *
            Mat3d(translation(cur_x, cur_y)));
    t1.fill(I, Color_name::red); t1.draw(I, Color_name::red);

    I.write_to("transformations.ppm");
}

void test_insert_image() {
    using namespace PPM_lib;
    constexpr int w {1600}, h {1200};
    RGB_Image I {w, h};

    RGB_Image J {"../imgs/baboon.ppm"};
    const auto img_w = J.width(), img_h = J.height();
    std::cout << img_w << ' ' << img_h << '\n';
    //insert_image(I, J);
    PPM_lib::Rectangle r {0, 0, img_w * 2, img_h * 2};
    fill_rect(r, I, J);

    I.write_to("img.ppm");
}

int main() {

    //test_transform();
    test_insert_image();

    return 0;
}

