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

void test_lines() {
    constexpr int w {300}, h {200};
    PPM_Image img {w, h, Color_name::blue};
    const Line l1 {Point{10, 10}, Point{20, 20}};
    Line l2 {l1};
    Line l3 {w >> 1, h >> 1, 200, 125};
    const Point p1 {200, 10}, p2 {10, 100};
    Line l4 = Line{p1, p2};

    l1.draw(img, PPM_Color{127});
    l2.draw(img, PPM_Color{127});
    l3.draw(img);
    l4.draw(img, Color_name::red);
    cout << "l4 length: " << l4.length() << ", area: " << l4.area() << endl;

    write_ppm_image(img, "output.ppm");
}

void test_model() {
    const Model m {"../obj/african_head.obj"};
    const int nv {m.num_vertices()}, nf {m.num_faces()};
    cout << "Number of vertices: " << nv << ", number of faces: " << nf << endl;

    constexpr int w {800}, h {800};
    PPM_Image img {w, h};

    for (int i {0}; i < nf; ++i) {
        const vector<int> f {m.face(i)};
        for (int j {0}; j < 3; ++j) {
            const vector<double> v1 {m.vertex(f[j])};
            const vector<double> v2 {m.vertex(f[(j + 1) % 3])};
            const Point p1 {static_cast<int>((v1[0] + 1) * (w >> 1)),
                static_cast<int>(h - (v1[1] + 1) * (h >> 1))};
            const Point p2 {static_cast<int>((v2[0] + 1) * (w >> 1)),
                static_cast<int>(h - (v2[1] + 1) * (h >> 1))};
            Line{p1, p2}.draw(img);
        }
    }
    write_ppm_image(img, "output.ppm");
}

int main() {

    //test_lines();
    test_model();

    return 0;
}

