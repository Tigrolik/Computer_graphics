/*
 * Set of codes adapted from https://github.com/ssloy/tinyrenderer/wiki/
 * The model used only for learning/educational/training purposes and is the
 * same as in the original source.
 */

#include "PPM_Image.h"
#include "Geometry.h"
#include "Model.h"
#include <iostream>
#include <iterator>
#include <thread>
#include <chrono>

using namespace std;

vector<double> diff(const vector<double> &v1, const vector<double> &v2) {
    return vector<double> {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}

double dot(const vector<double> &v1, const vector<double> &v2) {
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

vector<double> cross(const vector<double> &v1, const vector<double> &v2) {
    return vector<double> {
        v1[1] * v2[2] - v1[2] * v2[1], // y * z - z * y
        v1[2] * v2[0] - v1[0] * v2[2], // z * x - x * z
        v1[0] * v2[1] - v1[1] * v2[0], // x * y - y * x
    };
}

//double norm(const vector<double> &v) {
//    return sqrt(sqr_fun(v[0]) + sqr_fun(v[1]) + sqr_fun(v[2]));
//}

//vector<double>normalize(const vector<double> &v, const double p = 1) {
void normalize(vector<double> &v, const double p = 1) {
    const double n {p / sqrt(sqr_fun(v[0]) + sqr_fun(v[1]) + sqr_fun(v[2]))};
    v[0] *= n; v[1] *= n; v[2] *= n;
    //return vector<double> {v[0] * n, v[1] * n, v[2] * n};
}

void test_model() {
    const Model m {"../obj/african_head.obj"};
    //const int nv {m.num_vertices()}, nf {m.num_faces()};
    //cout << "Number of vertices: " << nv <<
    //", number of faces: " << nf << endl;

    constexpr int w {800}, h {800};
    PPM_Image img {w, h};

    // wireframe
    //for (int i {0}; i < nf; ++i) {
    //    const vector<int> f {m.face(i)};
    //    for (int j {0}; j < 3; ++j) {
    //        const vector<double> v1 {m.vertex(f[j])};
    //        const vector<double> v2 {m.vertex(f[(j + 1) % 3])};
    //        const Point p1 {static_cast<int>((v1[0] + 1) * (w >> 1)),
    //            static_cast<int>(h - (v1[1] + 1) * (h >> 1))};
    //        const Point p2 {static_cast<int>((v2[0] + 1) * (w >> 1)),
    //            static_cast<int>(h - (v2[1] + 1) * (h >> 1))};
    //        Line{p1, p2}.draw(img);
    //    }
    //}

    // random colors
    //for (int i {0}; i < nf; ++i) {
    //    const vector<int> f {m.face(i)};
    //    const vector<double> v1 {m.vertex(f[0])};
    //    const vector<double> v2 {m.vertex(f[1])};
    //    const vector<double> v3 {m.vertex(f[2])};
    //    const Point p1 {static_cast<int>((v1[0] + 1) * (w >> 1)),
    //        static_cast<int>(h - (v1[1] + 1) * (h >> 1))};
    //    const Point p2 {static_cast<int>((v2[0] + 1) * (w >> 1)),
    //        static_cast<int>(h - (v2[1] + 1) * (h >> 1))};
    //    const Point p3 {static_cast<int>((v3[0] + 1) * (w >> 1)),
    //        static_cast<int>(h - (v3[1] + 1) * (h >> 1))};
    //    const Triangle tri {p1, p2, p3};
    //    tri.fill(img, PPM_Color(rand() % 255, rand() % 255, rand() % 255));
    //    tri.draw(img, Color_name::black);
    //}

    // enlighted model
    const int w2 {w >> 1}, h2 {h >> 1};
    const vector<double> light_dir {0, 0, -1};
    for (int i {0}; i < m.num_faces(); ++i) {
        const vector<int> f {m.face(i)};
        const vector<double> v1 {m.vertex(f[0])};
        const vector<double> v2 {m.vertex(f[1])};
        const vector<double> v3 {m.vertex(f[2])};
        vector<double> n {cross(diff(v3, v1), diff(v2, v1))};
        normalize(n);
        //const double br {dot(n, light_dir)};
        const double br {-n[2]};

        const Triangle tri {static_cast<int>((v1[0] + 1) * w2),
            static_cast<int>(h - (v1[1] + 1) * h2), static_cast<int>((v2[0] +
                        1) * w2), static_cast<int>(h - (v2[1] + 1) * h2),
            static_cast<int>((v3[0] + 1) * (w >> 1)),
            static_cast<int>(h - (v3[1] + 1) * (h >> 1))
        };
        //const Point p1 {static_cast<int>((v1[0] + 1) * (w >> 1)),
        //    static_cast<int>(h - (v1[1] + 1) * (h >> 1))};
        //const Point p2 {static_cast<int>((v2[0] + 1) * (w >> 1)),
        //    static_cast<int>(h - (v2[1] + 1) * (h >> 1))};
        //const Point p3 {static_cast<int>((v3[0] + 1) * (w >> 1)),
        //    static_cast<int>(h - (v3[1] + 1) * (h >> 1))};
        //const Triangle tri {p1, p2, p3};
        if (br > 0)
            tri.fill(img, PPM_Color{static_cast<uchar>(br * 255)});
        //tri.draw(img, Color_name::yellow);
    }

    write_ppm_image(img, "output.ppm");
}

void test_triangle() {
    constexpr int w {800}, h {800};
    PPM_Image img {w, h};
    const Triangle tri {Point {50, 50}, Point {550, 150}, Point {300, 400}};
    tri.fill(img, Color_name::red);
    tri.draw(img, PPM_Color{24, 255, 24});
    const Triangle tri2 {Point {50, 400}, Point {50, 700}, Point {450, 700}};
    tri2.fill(img, PPM_Color{64});
    tri2.draw(img, Color_name::yellow);
    cout << "tri2 length: " << tri2.length() << ", area: " << tri2.area()
        << endl;

    write_ppm_image(img, "output.ppm");
}

int main() {

    test_model();
    //test_triangle();

    return 0;
}

