/*
 * Set of codes adapted from https://github.com/ssloy/tinyrenderer/wiki/
 * The model used only for learning/educational/training purposes and is the
 * same as in the original source.
 *
 */

#include "Model.h"
#include <iostream>

using namespace std;

/*
 * Calculate barycentric coordinates exploiting vector cross product
 * p1, p2 and p3 are the triangle corner points and p is a point for which we
 * are calculating the barycentric coordinates. The point p is not neccessarily
 * inside the triangle.
 * The point p1 with vectors p1p2 and p1p3 make a reference frame
 */
Vec3f barycentric(const Point &p1, const Point &p2, const Point &p3,
        const Point &p) {
    const int x1 {p1.x()}, y1 {p1.y()};
    Vec3f v {Vec3f(p3.x() - x1, p2.x() - x1, x1 - p.x()) ^
        Vec3f(p3.y() - y1, p2.y() - y1, y1 - p.y())};
    const double z {v.z()};
    if (std::abs(z) < 1) return Vec3f {-1, 1, 1};
    const double xz {v.x() / z}, yz {v.y() / z};
    return Vec3f {1 - xz - yz, yz, xz};
}

/*
 * Filling a triangle using barycentric coordinates:
 * 1. Make a framing box (rectangle)
 * 2. Calculate barycentric coordinates for each point inside the box
 * 3. If all barycentric coordinates are non-negative => inside the triangle
 */
void triangle_bar(const Point &p1, const Point &p2, const Point &p3,
        PPM_Image &I, const PPM_Color &c = 255) {
    std::array<Vec3i, 3> pts {Vec3i{p1.x(), p1.y()}, Vec3i{p2.x(), p2.y()},
        Vec3i{p3.x(), p3.y()}};
    Vec3i bboxmin{I.width() - 1, I.height() - 1}, bboxmax{0, 0}, clamp{bboxmin};
    for (int i {0}; i < 3; ++i)
        for (int j {0}; j < 2; ++j) {
            bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    for (int x {bboxmin.x()}; x <= bboxmax.x(); ++x)
        for (int y {bboxmin.y()}; y <= bboxmax.y(); ++y) {
            Vec3f bc_sc {barycentric(p1, p2, p3, Point{x, y})};
            if (bc_sc.x() >= 0 && bc_sc.y() >= 0 && bc_sc.z() >= 0)
                I.set_color(x, y, c);
        }
}

void test_model() {
    const Model m {"../obj/african_head.obj"};
    cout << "Verts: " << m.num_vertices() << ", v[1]: " << m.vertex(1) << endl;
    cout << "Norms: " << m.num_normals() << ", n[1]: " << m.normal(1,1) << endl;
    cout << "Texture vertices: " << m.num_texvertices() <<
        ", texv[1][1]: " << m.texvertex(1, 1) << endl;
    cout << "Faces: " << m.num_faces() << ", f[2]: " << m.face(2) << endl;
    Vec3f v {m.normal(1, 2)}, v2 {m.normal(2, 1)};
    cout << v + v2 << endl;
}

void test_camera() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, d {255};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};

    const Vec3f light_dir {Vec3f{1, -1, 1}.normalize()}, eye {1, 1, 3},
          center {0, 0, 0};
    const Matrix ModelView {Matrix::lookat(eye, center, Vec3f{0, 1, 0})};
    const Matrix VP {Matrix::viewport(w >> 3, h >> 3, (w >> 2) * 3,
            (h >> 2) * 3, d)};
    //const Matrix VP {viewport(w >> 3, h >> 3, (w >> 2) * 3, (h >> 2) * 3, d)};
    Matrix Proj {Matrix::identity()};
    Proj[3][2] = -1.0 / (eye - center).norm();
    const Matrix Z {VP * Proj * ModelView};
    //cout << ModelView << Proj << VP << Z;
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Vec3i f {m.face(i)};
        // world coordinates
        const array<Vec3f, 3> wc {m.vertex(f[0]), m.vertex(f[1]),
            m.vertex(f[2])};
        // screen coordinates
        array<Vec3i, 3> sc;
        // light intensity (kind of brightness)
        array<double, 3> br;
        for (int j {0}; j < 3; ++j) {
            sc[j] = Z * Matrix(wc[j]);
            sc[j][1] = h - sc[j].y(); // flip upside down
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

void test_bar() {
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};
    const Point p1 {-10, 10}, p2 {400, 100}, p3 {100, 550};
    triangle_bar(p1, p2, p3, I, {128, 240, 75});
    I.write_to("output.ppm");
}

int main() {

    //test_model();
    test_camera();
    //test_bar();

    return 0;
}

