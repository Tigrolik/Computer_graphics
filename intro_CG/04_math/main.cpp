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

// transformation matrix to the unit square
Mat3d unit_square_to_points(const PPM_lib::Point p1, const PPM_lib::Point p2,
        const PPM_lib::Point p3, const PPM_lib::Point p4) {
    const int x1 = p1.x(), x2 = p2.x(), x3 = p3.x(), x4 = p4.x();
    const int y1 = p1.y(), y2 = p2.y(), y3 = p3.y(), y4 = p4.y();
    const double dx1 = x2 - x3, dx2 = x4 - x3, dx3 = x1 - dx1 - x4;
    const double dy1 = y2 - y3, dy2 = y4 - y3, dy3 = y1 - dy1 - y4;
    using namespace Algebra_lib;
    const double det_aux = det(Mat<2, 2, double>{Vec<2, double>{dx1, dx2},
            Vec<2, double>{dy1, dy2}});
    const double a13 = det(Mat<2, 2, double>{Vec<2, double>{dx3, dx2},
            Vec<2, double>{dy3, dy2}}) / double(det_aux);
    const double a23 = det(Mat<2, 2, double>{Vec<2, double>{dx1, dx3},
            Vec<2, double>{dy1, dy3}}) / double(det_aux);
    const double a11 = x2 - x1 + a13 * x2, a21 = x4 - x1 + a23 * x4, a31 = x1;
    const double a12 = y2 - y1 + a13 * y2, a22 = y4 - y1 + a23 * y4, a32 = y1;
    return Mat3d {Vec3d{a11, a12, a13}, Vec3d{a21, a22, a23},
        Vec3d{a31, a32, 1}};
}

Mat3d unit_square_to_points(const std::vector<PPM_lib::Point> &vp) {
    if (vp.size() < 4)
        throw std::runtime_error {"vector of points is too small (need 4)"};
    return unit_square_to_points(vp[0], vp[1], vp[2], vp[3]);
}

// invert matrix
Mat3d invert_transform_mat(const Mat3d& M) {
    const double a = M[0][0], c = M[0][1], p = M[0][2];
    const double b = M[1][0], d = M[1][1], q = M[1][2];
    const double l = M[2][0], m = M[2][1];
    return Mat3d {Vec3d {d - q * m, m * p - c, c * q - p * d},
        Vec3d {q * l - b, a - p * l, p * b - a * q},
        Vec3d {b * m - d * l, l * c - a * m, a * d - c * b}};
}

// transformation matrix for four points to four points
Mat3d points_to_points(const std::vector<PPM_lib::Point> &vp1,
        const std::vector<PPM_lib::Point> &vp2) {
    const auto M1 = unit_square_to_points(vp1);
    const auto M1_inv = invert_transform_mat(M1);
    const auto M2 = unit_square_to_points(vp2);
    return M1_inv * M2;
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

// insert image into an RGB_Image
void insert_image(PPM_lib::RGB_Image &I, const PPM_lib::RGB_Image &J) {
    const auto w = J.width(), h = J.height();
    //PPM_lib::Rectangle r {0, 0, w, h};
    for (auto i = 0; i < std::min<int>(I.width(), w); ++i)
        for (auto j = 0; j < std::min<int>(I.height(), h); ++j)
            I[i][j] = J[i][j];
}

// interpolate grayscale color
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

// interpolate rgb color
PPM_lib::RGB_Color interp_rgb_color(const PPM_lib::RGB_Image& I,
        const PPM_lib::Point p1, const PPM_lib::Point p2,
        const PPM_lib::Point p3, const PPM_lib::Point p4,
        const double x, const double y) {
    const int min_x = p1.x(), max_x = p2.x(), min_y = p1.y(), max_y = p3.y();
    const PPM_lib::RGB_Color c1 = I.color(p1.x(), p1.y());
    const PPM_lib::RGB_Color c2 = I.color(p2.x(), p2.y());
    const PPM_lib::RGB_Color c3 = I.color(p3.x(), p3.y());
    const PPM_lib::RGB_Color c4 = I.color(p4.x(), p4.y());

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

void test_points_transform() {
    using namespace PPM_lib;
    constexpr int w {600}, h {400};
    RGB_Image I {w, h};

    Point p1 {10, 10}, p2 {120, 40}, p3 {150, 130}, p4 {45, 170};
    const std::vector<Point> vp {p1, p2, p3, p4};
    Polygon poly1 {p1, p2, p3, p4};
    // transformation matrix
    const auto M1 = unit_square_to_points(p1, p2, p3, p4);
    const auto M1_inv = invert_transform_mat(M1);
    std::cout << M1 << '\n';

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j) {
            auto v = Vec3d{double(i), double(j), 1} * M1;
            // normalize
            v[0] /= v[2];
            v[1] /= v[2];
            std::cout << "Point(" << i << ", " << j << "): " << v << '\n';
            v = Vec3d{point_to_vec(vp[i * 2 + j])} * M1_inv;
            v[0] /= v[2];
            v[1] /= v[2];
            std::cout << "Point " << i * 2 + j << ": " << v << '\n';
        }
    poly1.draw(I);

    Point p5 {100, 100}, p6 {220, 140}, p7 {250, 230}, p8 {145, 270};
    Polygon{p5, p6, p7, p8}.draw(I);

    const auto M2 = unit_square_to_points(p5, p6, p7, p8);
    const auto M = M1_inv * M2;
    for (size_t i {0}; i < vp.size(); ++i) {
        auto v = Vec3d{point_to_vec(vp[i])} * M;
        v[0] /= v[2];
        v[1] /= v[2];
        std::cout << "Point " << i << ": " << v << '\n';
    }

    I.write_to("points.ppm");
}

PPM_lib::Point point_in_poly(const std::vector<PPM_lib::Point> vp,
        const double t, const double s) {
    const int x1 = vp[0].x(), x2 = vp[1].x(), x3 = vp[2].x(), x4 =
        vp[3].x();
    const int y1 = vp[0].y(), y2 = vp[1].y(), y3 = vp[2].y(), y4 =
        vp[3].y();
    const int dx1 = x2 - x1, dx2 = x3 - x4, dx3 = x4 - x1, dx4 = x3 - x2;
    const int dy1 = y2 - y1, dy2 = y3 - y4, dy3 = y4 - y1, dy4 = y3 - y2;
    const int max_x = std::max({std::abs(dx1), std::abs(dx2), std::abs(dy1),
            std::abs(dy2)});
    const int max_y = std::max({std::abs(dx3), std::abs(dx4), std::abs(dy3),
            std::abs(dy4)});
    const double x_idx1 = x1 + dx1 * double(t) / max_x;
    const double y_idx1 = y1 + dy1 * double(t) / max_x;
    //Point{std::round(x_idx1), std::round(y_idx1)}.draw(I, Color_name::red);
    const double x_idx2 = x4 + dx2 * double(t) / max_x;
    const double y_idx2 = y4 + dy2 * double(t) / max_x;
    //Point{std::round(x_idx2), std::round(y_idx2)}.draw(I, Color_name::red);
    const double dx_idx = x_idx2 - x_idx1;
    const double dy_idx = y_idx2 - y_idx1;
    const double x_idx = x_idx1 + dx_idx * double(s) / max_y;
    const double y_idx = y_idx1 + dy_idx * double(s) / max_y;
    return PPM_lib::Point{std::round(x_idx), std::round(y_idx)};
}

/* --- Interpolation --- */

// interpolate color with floating point coordinates
unsigned char interp_value(const unsigned char c1, const unsigned char c2,
        const unsigned char c3, const unsigned char c4,
        const double x, const double y) {
    const auto x1 = int(x), x2 = x1 + 1;
    const auto y1 = int(y), y2 = y1 + 1;
    const auto dx1 = x - x1, dx2 = x2 - x;
    return (y2 - y) * (dx2 * c1 + dx1 * c2) + (y - y1) * (dx2 * c4 + dx1 * c3);
}

// interpolate rgb color value
PPM_lib::RGB_Color interp_rgb_value(const PPM_lib::RGB_Image& I,
        const double x, const double y) {
    const int x1 = int(x), x2 = x1 + 1;
    const int y1 = int(y), y2 = y1 + 1;
    const PPM_lib::RGB_Color c1 = I.color(x1, y1);
    const PPM_lib::RGB_Color c2 = I.color(x2, y1);
    const PPM_lib::RGB_Color c3 = I.color(x2, y2);
    const PPM_lib::RGB_Color c4 = I.color(x1, y2);
    const auto val1 = interp_value(c1.red(), c2.red(), c3.red(), c4.red(),
            x, y);
    const auto val2 = interp_value(c1.green(), c2.green(), c3.green(),
            c4.green(), x, y);
    const auto val3 = interp_value(c1.blue(), c2.blue(), c3.blue(), c4.blue(),
            x, y);

    return PPM_lib::RGB_Color{val1, val2, val3};
}

// fill a rectangular area with image
void fill_rect(const PPM_lib::Rectangle& R, PPM_lib::RGB_Image& img,
        const PPM_lib::RGB_Image& I) {
    const int w = I.width(), h = I.height();
    const int rw = R.width(), rh = R.height();
    const double xq = w / double(rw);
    const double yq = h / double(rh);
    using namespace PPM_lib;
    for (int i {0}; i < rw - rw / w; ++i)
        for (int j {0}; j < rh - rh / h; ++j) {
            PPM_lib::RGB_Color c = interp_rgb_value(I, xq * i, yq * j);
            img.set_color(i, j, c);
        }
}

/* --- End Interpolation --- */



void apply_warp(PPM_lib::RGB_Image& I, const PPM_lib::RGB_Image& J,
        const std::vector<PPM_lib::Point> &vp1,
        const std::vector<PPM_lib::Point> &vp2) {
    const int img_w = J.width(), img_h = J.height();
    const auto M1 = unit_square_to_points(vp1);
    const auto M2 = points_to_points(vp2, vp1);
    const int max_x = std::max({vp1[0].x(), vp1[1].x(), vp1[2].x(),
            vp1[3].x()});
    const int min_x = std::min({vp1[0].x(), vp1[1].x(), vp1[2].x(),
            vp1[3].x()});
    const int max_y = std::max({vp1[0].y(), vp1[1].y(), vp1[2].y(),
            vp1[3].y()});
    const int min_y = std::min({vp1[0].y(), vp1[1].y(), vp1[2].y(),
            vp1[3].y()});
    const int dx = max_x - min_x, dy = max_y - min_y;

    using namespace PPM_lib;

    for (int i {0}; i < dx; ++i)
        for (int j {0}; j < dy; ++j) {
            const double u = double(i) / dx;
            const double v = double(j) / dy;
            auto p = Vec3d{u, v, 1} * M1;
            p[0] /= p[2]; p[1] /= p[2];
            auto q = Vec3d{p[0], p[1], 1} * M2;
            q[0] /= q[2]; q[1] /= q[2];

            const int i1 = int(q[0]), i2 = i1 + 1;
            const int j1 = int(q[1]), j2 = j1 + 1;
            if (i2 < img_w && j2 < img_h) {
                PPM_lib::RGB_Color c = interp_rgb_color(J, Point{i1, j1},
                        Point{i2, j1}, Point{i2, j2}, Point{i1, j2},
                        q.x(), q.y());
                I.set_color(p[0], p[1], c);
            }
        }
}

void apply_warp2(PPM_lib::RGB_Image& I, const PPM_lib::RGB_Image& J,
        const std::vector<PPM_lib::Point> &vp1,
        const std::vector<PPM_lib::Point> &vp2) {

    const int img_w = J.width(), img_h = J.height();

    const auto M = points_to_points(vp1, vp2);

    const int x1 = vp1[0].x(), x2 = vp1[1].x(), x3 = vp1[2].x(),
          x4 = vp1[3].x();
    const int y1 = vp1[0].y(), y2 = vp1[1].y(), y3 = vp1[2].y(),
          y4 = vp1[3].y();
    const int dx1 = x2 - x1, dx2 = x3 - x4, dx3 = x4 - x1, dx4 = x3 - x2;
    const int dy1 = y2 - y1, dy2 = y3 - y4, dy3 = y4 - y1, dy4 = y3 - y2;
    const int max_x = std::max({std::abs(dx1), std::abs(dx2), std::abs(dy1),
            std::abs(dy2)});
    const int max_y = std::max({std::abs(dx3), std::abs(dx4), std::abs(dy3),
            std::abs(dy4)});
    using namespace PPM_lib;
    for (int i {0}; i < max_x; ++i) {
        const double x_idx1 = x1 + dx1 * double(i) / max_x;
        const double y_idx1 = y1 + dy1 * double(i) / max_x;
        const double x_idx2 = x4 + dx2 * double(i) / max_x;
        const double y_idx2 = y4 + dy2 * double(i) / max_x;
        const double dx_idx = x_idx2 - x_idx1;
        const double dy_idx = y_idx2 - y_idx1;
        for (int j {0}; j < max_y; ++j) {
            const double x_idx = x_idx1 + dx_idx * double(j) / max_y;
            const double y_idx = y_idx1 + dy_idx * double(j) / max_y;
            const double a = double(i) / max_x;
            const double b = double(j) / max_y;
            const double x_temp = x1 + dx1 * a + dx3 * b + (x1 - x2 + x3 - x4) *
                a * b;
            const double y_temp = y1 + dy1 * a + dy3 * b + (y1 - y2 + y3 - y4) *
                a * b;
            //Vec3d p = Vec3d{x_idx, y_idx, 1} * M;
            Vec3d p = Vec3d{x_temp, y_temp, 1} * M;
            p[0] /= p[2]; p[1] /= p[2];
            const int i1 = std::floor(p[0]), i2 = i1 + 1;
            const int j1 = std::floor(p[1]), j2 = j1 + 1;
            //const int i1 = std::floor(x_idx), i2 = i1 + 1;
            //const int j1 = std::floor(y_idx), j2 = j1 + 1;
            if (i1 >= 0 && j1 >= 0 && i2 < img_w && j2 < img_h) {
                PPM_lib::RGB_Color c = interp_rgb_value(J, p[0], p[1]);
                //Point{std::round(x_idx), std::round(y_idx)}.draw(I, c);
                //I.set_color(std::round(p[0]), std::round(p[1]), c);
                I[x_idx][y_idx] = c.color();
            }
        }
    }
}

// warping loop
void warp_loop4squares(PPM_lib::RGB_Image& I, const PPM_lib::RGB_Image& J,
        const std::vector<PPM_lib::Point> &vp1, const int x_off = 0,
        const int y_off = 0) {
    const int tex_img_w = J.width(), tex_img_h = J.height();
    const int half_w = tex_img_w >> 1, half_h = tex_img_h >> 1;
    const int x1 = vp1[0].x(), x2 = vp1[1].x(), x3 = vp1[2].x(),
          x4 = vp1[3].x();
    const int y1 = vp1[0].y(), y2 = vp1[1].y(), y3 = vp1[2].y(),
          y4 = vp1[3].y();
    const int xa = x2 - x1, xb = x4 - x1, xab = x1 - x2 + x3 - x4;
    const int ya = y2 - y1, yb = y4 - y1, yab = y1 - y2 + y3 - y4;

    for (int i {0}; i < half_w; ++i)
        for (int j {0}; j < half_h; ++j) {
            const double a = double(i) / half_w;
            const double b = double(j) / half_h;
            const double x_out = x1 + xa * a + xb * b + xab * a * b;
            const double y_out = y1 + ya * a + yb * b + yab * a * b;
            if (x_out >= 0 && x_out < tex_img_w - 1 && y_out >= 0 &&
                    y_out < tex_img_h - 1) {
                PPM_lib::RGB_Color clr = interp_rgb_value(J, x_out, y_out);
                I[x_off + i][y_off + j] = clr.color();
            }
        }
}

void warp_loop9squares(PPM_lib::RGB_Image& I, const PPM_lib::RGB_Image& J,
        const std::vector<PPM_lib::Point> &vp1, const int x_off = 0,
        const int y_off = 0) {
    const int tex_img_w = J.width(), tex_img_h = J.height();
    const int tri_w = tex_img_w / 3, tri_h = tex_img_h / 3;
    const int x1 = vp1[0].x(), x2 = vp1[1].x(), x3 = vp1[2].x(),
          x4 = vp1[3].x();
    const int y1 = vp1[0].y(), y2 = vp1[1].y(), y3 = vp1[2].y(),
          y4 = vp1[3].y();
    const int xa = x2 - x1, xb = x4 - x1, xab = x1 - x2 + x3 - x4;
    const int ya = y2 - y1, yb = y4 - y1, yab = y1 - y2 + y3 - y4;

    for (int i {0}; i < tri_w; ++i)
        for (int j {0}; j < tri_h; ++j) {
            const double a = double(i) / tri_w;
            const double b = double(j) / tri_h;
            const double x_out = x1 + xa * a + xb * b + xab * a * b;
            const double y_out = y1 + ya * a + yb * b + yab * a * b;
            if (x_out >= 0 && x_out < tex_img_w - 1 && y_out >= 0 &&
                    y_out < tex_img_h - 1) {
                PPM_lib::RGB_Color clr = interp_rgb_value(J, x_out, y_out);
                I[x_off + i][y_off + j] = clr.color();
            }
        }
}

void test_image_warp4regions() {
    using namespace PPM_lib;
    constexpr int w {1600}, h {1200};
    RGB_Image I {w, h};

    const RGB_Image J {"../imgs/baboon.ppm"};
    const int img_w = J.width(), img_h = J.height();
    const int hw = img_w >> 1, th = img_h / 2;

    const Point p {hw - 50, th - 50}; // "control" point
    const std::vector<Point> vp {{0, 0}, {hw, 0}, p, {0, th}};
    warp_loop4squares(I, J, vp);
    warp_loop4squares(I, J, {{0, th}, p, {hw, img_h}, {0, img_h}}, 0, th);
    warp_loop4squares(I, J, {{hw, 0}, {img_w, 0}, {img_w, th}, p}, hw, 0);
    warp_loop4squares(I, J, {p, {img_w, th}, {img_w, img_h}, {hw, img_h}}, hw,
            th);

    I.write_to("warping.ppm");
}

void test_image_warp9regions() {
    using namespace PPM_lib;
    constexpr int w {1600}, h {1200};
    RGB_Image I {w, h};

    const RGB_Image J {"../imgs/baboon.ppm"};
    const int img_w = J.width(), img_h = J.height();
    const int w_tri = img_w / 3, h_tri = img_h / 3;

    //PPM_lib::Rectangle r {0, 0, size_t(img_w), size_t(img_h)};
    //fill_rect(r, I, J);

    const Point p1 {w_tri - 40, h_tri - 50};
    const Point p2 {2 * w_tri - 20, h_tri - 40};
    const Point p3 {w_tri - 30, 2 * h_tri + 30};
    const Point p4 {2 * w_tri + 30, 2 * h_tri - 40};
    warp_loop9squares(I, J, {{0, 0}, {w_tri, 0}, p1, {0, h_tri}});
    warp_loop9squares(I, J, {{w_tri, 0}, {2 * w_tri, 0}, p2, p1}, w_tri);
    warp_loop9squares(I, J, {{2 * w_tri, 0}, {img_w, 0}, {img_w, h_tri}, p2},
            2 * w_tri);
    warp_loop9squares(I, J, {{0, h_tri}, p1, p3, {0, 2 * h_tri}}, 0, h_tri);
    warp_loop9squares(I, J, {p1, p2, p4, p3}, w_tri, h_tri);
    warp_loop9squares(I, J, {p2, {img_w, h_tri}, {img_w, 2 * h_tri}, p4},
            2 * w_tri, h_tri);
    warp_loop9squares(I, J, {{0, 2 * h_tri}, p3, {w_tri, img_h}, {0, img_h}},
            0, 2 * h_tri);
    warp_loop9squares(I, J, {p3, p4, {2 * w_tri, img_h}, {w_tri, img_h}},
            w_tri, 2 * h_tri);
    warp_loop9squares(I, J, {p4, {img_w, 2 * h_tri}, {img_w, img_h},
            {2 * w_tri, img_h}}, 2 * w_tri, 2 * h_tri);

    I.write_to("warping.ppm");
}

void test_points_again() {
    using namespace PPM_lib;
    using namespace Algebra_lib;
    std::vector<Point> vp1 {{0, 0}, {3, 1}, {4, 3}, {2, 2}};
    std::vector<Point> vp2 {{2, 1}, {6, 7}, {8, 13}, {-2, 9}};
    const auto M = points_to_points(vp1, vp2);
    std::cout << M << '\n';
    auto t = dot(Vec3d{0, 0, 1}, M.col(2));
    std::cout << dot(Vec3d{0, 0, 1}, M.col(0)) / t << ' ';
    std::cout << dot(Vec3d{0, 0, 1}, M.col(1)) / t << '\n';
    t = dot(Vec3d{3, 1, 1}, M.col(2));
    std::cout << dot(Vec3d{3, 1, 1}, M.col(0)) / t << ' ';
    std::cout << dot(Vec3d{3, 1, 1}, M.col(1)) / t << '\n';
    t = dot(Vec3d{4, 3, 1}, M.col(2));
    std::cout << dot(Vec3d{4, 3, 1}, M.col(0)) / t << ' ';
    std::cout << dot(Vec3d{4, 3, 1}, M.col(1)) / t << '\n';
    t = dot(Vec3d{2, 2, 1}, M.col(2));
    std::cout << dot(Vec3d{2, 2, 1}, M.col(0)) / t << ' ';
    std::cout << dot(Vec3d{2, 2, 1}, M.col(1)) / t << '\n';
}

void test_insert_image() {
    using namespace PPM_lib;
    constexpr int w {1600}, h {1200};
    RGB_Image I {w, h};

    RGB_Image J {"../imgs/baboon.ppm"};
    const auto img_w = J.width(), img_h = J.height();
    //PPM_lib::Rectangle r {0, 0, img_w * 5 / 7, img_h * 5 / 7};
    PPM_lib::Rectangle r {0, 0, img_w * 2, img_h * 2};
    fill_rect(r, I, J);

    I.write_to("img.ppm");
}

void test_poly_scan() {
    using namespace PPM_lib;
    constexpr int w {1600}, h {1200};
    RGB_Image I {w, h};

    Point p1 {10, 10}, p2 {120, 80}, p3 {150, 130}, p4 {45, 270};
    std::vector<Point> vp1 {p1, p2, p3, p4};
    Polygon{vp1}.draw(I);

    const int x1 = vp1[0].x(), x2 = vp1[1].x(), x3 = vp1[2].x(), x4 =
        vp1[3].x();
    const int y1 = vp1[0].y(), y2 = vp1[1].y(), y3 = vp1[2].y(), y4 =
        vp1[3].y();
    const int dx1 = x2 - x1, dx2 = x3 - x4, dx3 = x4 - x1, dx4 = x3 - x2;
    const int dy1 = y2 - y1, dy2 = y3 - y4, dy3 = y4 - y1, dy4 = y3 - y2;
    const int max_x = std::max({std::abs(dx1), std::abs(dx2), std::abs(dy1),
            std::abs(dy2)});
    const int max_y = std::max({std::abs(dx3), std::abs(dx4), std::abs(dy3),
            std::abs(dy4)});
    const auto clr = Color_name::red;
    for (int i {0}; i < max_x; ++i) {
        const double x_idx1 = x1 + dx1 * double(i) / max_x;
        const double y_idx1 = y1 + dy1 * double(i) / max_x;
        const double x_idx2 = x4 + dx2 * double(i) / max_x;
        const double y_idx2 = y4 + dy2 * double(i) / max_x;
        const double dx_idx = x_idx2 - x_idx1;
        const double dy_idx = y_idx2 - y_idx1;
        for (int j {0}; j < max_y; ++j) {
            const double x_idx = x_idx1 + dx_idx * double(j) / max_y;
            const double y_idx = y_idx1 + dy_idx * double(j) / max_y;
            Point{std::round(x_idx), std::round(y_idx)}.draw(I, clr);
        }
    }

    I.write_to("scan.ppm");
}

int main() {

    //test_transform();
    //test_insert_image();
    //test_points_transform();
    //test_image_warp4regions();
    test_image_warp9regions();
    //test_poly_scan();


    return 0;
}

