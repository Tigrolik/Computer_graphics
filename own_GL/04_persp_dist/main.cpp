/*
 * Set of codes adapted from https://github.com/ssloy/tinyrenderer/wiki/
 * The model used only for learning/educational/training purposes and is the
 * same as in the original source.
 *
 * In this exercise classes Vertex, Normal and Point3 are used instead of Vec3f,
 * Vec2i from the original source. It turns out that names Vertex, Normal and
 * Point3 are confusing (there is also Point class in Geometry), thus in the
 * next exercise I shall create Vec3 templated class to have hierarchy similar
 * to the original source (Vec3f and Vec3i)
 */

#include "Model.h"
#include <iostream>

using namespace std;

void test_init() {
    //constexpr int n {3};
    //constexpr std::array<int, n> arr1 {1, 2, 3};
    //const Vec_base<int, n> vb1;
    //const Vec_base<int, n> vb1 {arr1};
    //std::cout << vb1[1] << '\n';
    const Vertex vx1 {1.05, 2.5, 3.4};
    Vertex vx2 {vx1};
    std::cout << vx1[1] << '\n';
    std::cout << vx1.x() << '\n';
    std::cout << vx1 + vx2 << '\n';
    Point3 p1 {1, 2, 3};
    std::cout << p1 + Point3 {4, 5, 6} << '\n';
    std::cout << p1 + vx1 << '\n';
    Point3 p2 = p1 + vx1;
    std::cout << p2 << '\n';
    //std::array<int, 3> a1 = std::array<int, 3> {1, 2, 3};
    Point3 p3;
    std::cout << p3 << '\n';
    Vertex v3;
    std::cout << v3 << '\n';
    std::cout << v3.size() << '\n';
    Normal n1;
    std::cout << n1 << '\n';
}

void test_matrix() {
    Matrix m1 {3, 3, 1}, m2 {3, 3, 2}, m3 {m1 * m2};
    std::cout << m3;
    Matrix m4 {2, 3, 1}, m5 {3, 2};
    m4[0][1] = 2; m4[1][0] = 4; m4[1][1] = 3; m4[1][2] = 5;
    m5[0][0] = 2; m5[1][0] = 4; m5[1][1] = 3; m5[2][0] = 1; m5[2][1] = 6;
    std::cout << m4 * m5;
    std::cout << m4 << m4.transpose();
}

void test_model() {
    using namespace std;
    Model m {"../obj/african_head.obj"};
    cout << "Verts: " << m.num_vertices() << ", v[1]: " << m.vertex(1) << endl;
    cout << "Norms: " << m.num_normals() << ", n[1]: " << m.normal(1,1) << endl;
    cout << "Texture vertices: " << m.num_texvertices() <<
        ", texv[1][1]: " << m.texvertex(1, 1) << endl;
    cout << "Faces: " << m.num_faces() << ", f[1]: " << m.face(1) << endl;
    Normal v {m.normal(1, 2)}, v2 {m.normal(2, 1)};
    cout << v + v2 << endl;
}

void test_render() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800};
    PPM_Image img {w, h};

    // build some lines
    vector<Point> pv {{20, h - 766}, {744, h - 400}, {120, h - 336},
        {444, h- 400}, {330, h- 337}, {594, h- 600}, {10, h- 790},
        {790, h- 790}};
    Line{20, 766, 744, 400}.draw(img, Color_name::red);
    Line{120, 366, 444, 400}.draw(img, Color_name::green);
    Line{330, 337, 594, 600}.draw(img, Color_name::blue);
    Line{10, 790, 790, 790}.draw(img);

    img.write_to("lines.ppm");

    // make an image 16 pixels high to make the result visible
    PPM_Image ren {w, 16};
    vector<int> ybuf (w, 0);
    rasterize(pv[0], pv[1], ybuf, ren, Color_name::red);
    rasterize(pv[2], pv[3], ybuf, ren, Color_name::green);
    rasterize(pv[4], pv[5], ybuf, ren, Color_name::blue);

    ren.write_to("render.ppm");
}

void test_enlighted_model() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800};
    PPM_Image img {w, h};

    // enlighted model
    const int w2 {w >> 1}, h2 {h >> 1};
    const Normal light_dir {0, 0, -1};
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Point3 f {m.face(i)};
        // world coordinates (wc)
        array<Vertex, 3> wc {m.vertex(f[0]), m.vertex(f[1]), m.vertex(f[2])};
        // screen coordinates (sc)
        array<Point, 3> sc;
        for (int j {0}; j < 3; ++j)
            sc[j] = Point {(wc[j].x() + 1) * w2, (h - (wc[j].y() + 1) * h2)};
        Normal n {(wc[2] - wc[0]) ^ (wc[1] - wc[0])};
        n.normalize();
        const double br {n * light_dir};
        //const double br {-n[2]};
        const Triangle tri {sc[0], sc[1], sc[2]};
        if (br > 0)
            tri.fill(img, br * 255);
        //tri.draw(img, Color_name::yellow);
    }
    img.write_to("output.ppm");
}

void test_zbuffer() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, depth {255};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};

    const int w2 {w >> 1}, h2 {h >> 1}, d2 {depth >> 1};
    const Normal light_dir {0, 0, -1};
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Point3 f {m.face(i)};
        // world coordinates (wc)
        array<Vertex, 3> wc {m.vertex(f[0]), m.vertex(f[1]), m.vertex(f[2])};
        // screen coordinates (sc)
        array<Point3, 3> sc;
        for (int j {0}; j < 3; ++j)
            sc[j] = Point3 {(wc[j].x() + 1) * w2, (h - (wc[j].y() + 1) * h2),
                (wc[j].z() + 1) * d2};
        Normal n {(wc[2] - wc[0]) ^ (wc[1] - wc[0])};
        n.normalize();
        const double br {n * light_dir};
        if (br > 0)
            triangle_buf(sc[0], sc[1], sc[2], zbuf, img, br * 255);
    }
    img.write_to("output.ppm");

    PPM_Image zbimg {w, h};
    for (int i {0}; i < w; ++i)
        for (int j {0}; j < h; ++j)
            zbimg.set_color(i, j, zbuf[i + j * w]);
    zbimg.write_to("zbuffer.ppm");
}

void test_texture() {
    const Model m {"../obj/african_head.obj"};
    PPM_Image teximg {"../obj/african_head_diffuse.ppm"};
    constexpr int w {800}, h {800}, depth {255};
    const int tex_w {teximg.width()}, tex_h {teximg.height()};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};
    const int w2 {w >> 1}, h2 {h >> 1}, d2 {depth >> 1};
    const Normal light_dir {0, 0, -1};
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Point3 f {m.face(i)};
        array<Vertex, 3> wc {m.vertex(f[0]), m.vertex(f[1]), m.vertex(f[2])};
        array<Point3, 3> sc;
        for (int j {0}; j < 3; ++j)
            sc[j] = Point3 {(wc[j].x() + 1) * w2, (h - (wc[j].y() + 1) * h2),
                (wc[j].z() + 1) * d2};
        Normal n {(wc[2] - wc[0]) ^ (wc[1] - wc[0])};
        n.normalize();
        const double br {n * light_dir};
        Vertex uv1 {m.texvertex(i, 0)}, uv2 {m.texvertex(i, 1)},
               uv3 {m.texvertex(i, 2)};
        if (br > 0)
            triangle_tex(sc[0], sc[1], sc[2],
                    {uv1.x() * tex_w, uv1.y() * tex_h},
                    {uv2.x() * tex_w, uv2.y() * tex_h},
                    {uv3.x() * tex_w, uv3.y() * tex_h}, zbuf, img, teximg, br);
    }
    img.write_to("texmodel.ppm");
}

void test_transform() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, depth {255};
    const Matrix VP {viewport(w >> 2, h >> 2, w >> 1, h >> 1, depth)};
    PPM_Image img {w, h};
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Point3 f {m.face(i)};
        for (size_t j {0}; j < f.size(); ++j) {
            const Vertex wp0 {m.vertex(f[j])};
            const Vertex wp1 {m.vertex(f[(j + 1) % f.size()])};
            // draw the original model
            Vertex sp0 {m2v(VP * v2m(wp0))};
            Vertex sp1 {m2v(VP * v2m(wp1))};
            Line(sp0.x(), h - sp0.y(), sp1.x(), h - sp1.y()).draw(img);
            // draw the deformed (zoomed) model
            const Matrix T {zoom(1.5)};
            sp0 = m2v(VP * T * v2m(wp0));
            sp1 = m2v(VP * T * v2m(wp1));
            Line(sp0.x(), h - sp0.y(), sp1.x(), h - sp1.y()).draw(img,
                    Color_name::yellow);
        }
    }
    // drawing axes
    Vertex orig(0, 0, 0), x(1, 0, 0), y(0, 1, 0);
    orig = m2v(VP * v2m(orig));
    x = m2v(VP * v2m(x));
    y = m2v(VP * v2m(y));
    Line(orig.x(), h - orig.y(), x.x(), h - x.y()).draw(img, Color_name::red);
    Line(orig.x(), h - orig.y(), y.x(), h - y.y()).draw(img, Color_name::green);

    img.write_to("lines.ppm");
}

void test_view() {
    const Model m {"../obj/african_head.obj"};
    PPM_Image teximg {"../obj/african_head_diffuse.ppm"};
    constexpr int w {800}, h {800}, d {255};
    const int tex_w {teximg.width()}, tex_h {teximg.height()};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};
    //const int w2 {w >> 1}, h2 {h >> 1}, d2 {d >> 1};
    const Normal light_dir {0, 0, -1}, cam {0, 0, 3};

    const Matrix VP {viewport(w >> 3, h >> 3, (w >> 2) * 3, (h >> 2) * 3, d)};
    Matrix Proj {Matrix::identity(4)};
    Proj[3][2] = -1 / cam.z();
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Point3 f {m.face(i)};
        array<Vertex, 3> wc {m.vertex(f[0]), m.vertex(f[1]), m.vertex(f[2])};
        array<Point3, 3> sc;
        for (int j {0}; j < 3; ++j) {
            const Point3 p {m2v(VP * Proj * v2m(wc[j]))};
            sc[j] = Point3 {p.x(), h - p.y(), p.z()};
        }
        Normal n {(wc[2] - wc[0]) ^ (wc[1] - wc[0])};
        n.normalize();
        const double br {n * light_dir};
        Vertex uv1 {m.texvertex(i, 0)}, uv2 {m.texvertex(i, 1)},
               uv3 {m.texvertex(i, 2)};
        if (br > 0)
            triangle_tex(sc[0], sc[1], sc[2],
                    {uv1.x() * tex_w, uv1.y() * tex_h},
                    {uv2.x() * tex_w, uv2.y() * tex_h},
                    {uv3.x() * tex_w, uv3.y() * tex_h}, zbuf, img, teximg, br);
    }

    img.write_to("persp.ppm");
}

void test_camera() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, d {255};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};

    Normal light_dir {1, -1, 1}; light_dir.normalize();
    const Normal eye {1, 1, 3}, center {0, 0, 0};
    const Matrix ModelView {lookat(eye, center, Vertex {0, 1, 0})};
    const Matrix VP {viewport(w >> 3, h >> 3, w * 3 / 4, h * 3 / 4, d)};
    //const Matrix VP {viewport(w >> 3, h >> 3, (w >> 2) * 3, (h >> 2) * 3, d)};
    Matrix Proj {Matrix::identity(4)};
    Proj[3][2] = -1.0 / Normal(eye - center).norm();
    const Matrix Z {VP * Proj * ModelView};
    //cout << ModelView << Proj << VP << Z;
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Point3 f {m.face(i)};
        array<Vertex, 3> wc {m.vertex(f[0]), m.vertex(f[1]), m.vertex(f[2])};
        array<Point3, 3> sc;
        array<double, 3> br;
        for (int j {0}; j < 3; ++j) {
            const Point3 p {m2v(VP * Proj * ModelView * v2m(wc[j]))};
            //const Point3 p {m2v(Z * v2m(wc[j]))};
            sc[j] = Point3 {p.x(), h - p.y(), p.z()};
            br[j] = m.normal(i, j) * light_dir;
        }
        triangle_ref(sc[0], sc[1], sc[2], br[0], br[1], br[2], zbuf, img);
    }
    img.write_to("gouraud.ppm");

    PPM_Image zbimg {w, h};
    for (int i {0}; i < w; ++i)
        for (int j {0}; j < h; ++j)
            zbimg.set_color(i, j, zbuf[i + j * w]);
    zbimg.write_to("zbuffer.ppm");
}

int main() {

    //test_init();
    //test_matrix();
    //test_model();
    //test_render();
    //test_enlighted_model();
    //test_zbuffer();
    //test_texture();
    //test_transform();
    //test_view();
    test_camera();

    return 0;
}

