/*
 * Set of codes adapted from https://github.com/ssloy/tinyrenderer/wiki/
 * The model used only for learning/educational/training purposes and is the
 * same as in the original source.
 *
 * In this exercise I use std::vectors instead Vec3f, Vec2i from the original
 * source. The reason is to practice C++ containers and different approach
 */

#include "PPM_Image.h"
#include "Geometry.h"
#include "Model.h"
#include <iostream>

using namespace std;

void test_vectors() {
    const vector<double> v1 {1.3, 3.708, -5.23};
    const vector<double> v2 {-2.350, 4.9, 6.201};
    vector<double> v = v1 - v2;
    double d = v1 * v2;
    //const Vertex vx1 {1.3, 3.708, -5.23};
    //const Vertex vx2 {-2.350, 4.29, 6.201};
    //const Vertex vx = vx1 + vx2;
    for (const auto x: v)
        cout << x << ' ';
    cout << '\n';
    cout << d << '\n';
}


void test_model() {
    const Model m {"../obj/african_head.obj"};
    //const int nv {m.num_vertices()}, nf {m.num_faces()};
    //cout << "Number of vertices: " << nv <<
    //", number of faces: " << nf << endl;

    constexpr int w {800}, h {800};
    PPM_Image img {w, h};

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
        if (br > 0)
            tri.fill(img, PPM_Color{static_cast<uchar>(br * 255)});
        //tri.draw(img, Color_name::yellow);
    }

    write_ppm_image(img, "output.ppm");
}

void test_zbuffer() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, depth {255};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};
    const int w2 {w >> 1}, h2 {h >> 1}, d2 {depth >> 1};
    const vector<double> light_dir {0, 0, -1};
    for (int i {0}; i < m.num_faces(); ++i) {
        const vector<int> f {m.face(i)};
        const vector<double> v1 {m.vertex(f[0])};
        const vector<double> v2 {m.vertex(f[1])};
        const vector<double> v3 {m.vertex(f[2])};
        vector<int> p1 {static_cast<int>((v1[0] + 1) * w2),
            static_cast<int>(h - (v1[1] + 1) * h2),
            static_cast<int>((v1[2] + 1) * d2)};
        vector<int> p2 {static_cast<int>((v2[0] + 1) * w2),
            static_cast<int>(h - (v2[1] + 1) * h2),
            static_cast<int>((v2[2] + 1) * d2)};
        vector<int> p3 {static_cast<int>((v3[0] + 1) * w2),
            static_cast<int>(h - (v3[1] + 1) * h2),
            static_cast<int>((v3[2] + 1) * d2)};
        vector<double> n {cross(diff(v3, v1), diff(v2, v1))};
        normalize(n);
        //const double br {dot(n, light_dir)};
        const double br {-n[2]};
        if (br > 0)
            triangle_buf(p1, p2, p3, zbuf, img,
                    PPM_Color{static_cast<uchar>(br * 255)});
    }
    write_ppm_image(img, "output.ppm");

    PPM_Image zbimg {w, h};
    for (int i {0}; i < w; ++i)
        for (int j {0}; j < h; ++j)
            zbimg.pixel_color(i, j,
                    PPM_Color{static_cast<uchar>(zbuf[i + j * w])});
    write_ppm_image(zbimg, "zbuffer.ppm");
}

void test_render() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800};
    PPM_Image img {w, h};

    vector<Point> pv {{20, h - 766}, {744, h - 400}, {120, h - 336},
        {444, h- 400}, {330, h- 337}, {594, h- 600}, {10, h- 790},
        {790, h- 790}};
    Line{20, 766, 744, 400}.draw(img, Color_name::red);
    Line{120, 366, 444, 400}.draw(img, Color_name::green);
    Line{330, 337, 594, 600}.draw(img, Color_name::blue);
    Line{10, 790, 790, 790}.draw(img);

    //const int w2 {w >> 1}, h2 {h >> 1};
    write_ppm_image(img, "output.ppm");

    PPM_Image ren {w, 16};
    vector<int> ybuf (w, 0);
    rasterize(pv[0], pv[1], ybuf, ren, Color_name::red);
    rasterize(pv[2], pv[3], ybuf, ren, Color_name::green);
    rasterize(pv[4], pv[5], ybuf, ren, Color_name::blue);

    write_ppm_image(ren, "render.ppm");
}

void test_texture() {
    const Model m {"../obj/african_head.obj"};
    PPM_Image teximg {read_ppm_image("../obj/african_head_diffuse.ppm")};
    constexpr int w {800}, h {800}, depth {255};
    const int tex_w {teximg.width()}, tex_h {teximg.height()};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};
    const int w2 {w >> 1}, h2 {h >> 1}, d2 {depth >> 1};
    const vector<double> light_dir {0, 0, -1};
    for (int i {0}; i < m.num_faces(); ++i) {
        const vector<int> f {m.face(i)};
        const vector<double> v1 {m.vertex(f[0])};
        const vector<double> v2 {m.vertex(f[1])};
        const vector<double> v3 {m.vertex(f[2])};
        vector<int> p1 {static_cast<int>((v1[0] + 1) * w2),
            static_cast<int>(h - (v1[1] + 1) * h2),
            static_cast<int>((v1[2] + 1) * d2)};
        vector<int> p2 {static_cast<int>((v2[0] + 1) * w2),
            static_cast<int>(h - (v2[1] + 1) * h2),
            static_cast<int>((v2[2] + 1) * d2)};
        vector<int> p3 {static_cast<int>((v3[0] + 1) * w2),
            static_cast<int>(h - (v3[1] + 1) * h2),
            static_cast<int>((v3[2] + 1) * d2)};
        vector<double> n {cross(diff(v3, v1), diff(v2, v1))};
        normalize(n);
        //const double br {dot(n, light_dir)};
        const double br {-n[2]};
        vector<double> uv1 {m.texvertex(i, 0)}, uv2 {m.texvertex(i, 1)};
        vector<double> uv3 {m.texvertex(i, 2)};
        uv1[0] *= tex_w; uv1[1] *= tex_h; uv2[0] *= tex_w; uv2[1] *= tex_h;
        uv3[0] *= tex_w; uv3[1] *= tex_h;
        if (br > 0)
            triangle_tex(p1, p2, p3, round(uv1), round(uv2), round(uv3),
                    zbuf, img, teximg, br);
    }
    write_ppm_image(img, "texmodel.ppm");
}

void test_reading() {
    //const string fn {"output.ppm"};
    const string fn {"../obj/african_head_diffuse.ppm"};
    PPM_Image img {read_ppm_image(fn)};
    write_ppm_image(img, "test.ppm");
}

int main() {

    //test_vectors();
    //test_model();
    //test_render();
    //test_zbuffer();
    test_texture();
    //test_reading();

    return 0;
}

