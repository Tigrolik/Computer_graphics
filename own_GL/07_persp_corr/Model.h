/*
 * Current Model approach:
 * class Vec3 is a template class for arrays of fixed size.
 * The purpose is to create objects with 3 elements arrays of int or double
 *
 * Note: Vec3<int> and Vec3<double> declarations are given in the end of the
 * corresponding .cpp file
 *
 * With comparison to the original source I have the same names for Vec3 types,
 * however, the implementation details have subtle differences though (perhaps)
 * not signficant
 * Class Matrix is basically the same as in the original source. Functions
 * zoom(), lookat(), viewport() and others are implemented static alongside the
 * identity() function from the original source with aim to have them all in one
 * class and place
 */

#ifndef MODEL_H
#define MODEL_H

#include "PPM_Image.h"
#include "Geometry.h"
#include "Vec.h"
#include <fstream>
#include <array>
#include <vector>

/*
 * ------------------ Facet ------------------
 */
class Facet {
public:
    using Vec3i = Vec<3, int>;

    Facet();
    Facet(const Vec3i&, const Vec3i&, const Vec3i&);

    Vec3i& operator[](const int i) { return f_[i]; }
    const Vec3i& operator[](const int i) const { return f_[i]; }

    friend std::istream& operator>>(std::istream&, Facet&);

private:
    std::array<Vec3i, 3> f_;
};

/*
 * ------------------ Model ------------------
 */
class Model {
public:
    using Vec3i = Vec<3, int>;
    using Vec3d = Vec<3, double>;
    Model(const std::string&);

    ~Model() = default;

    const Vec3d vertex(const int i) const { return verts_[i]; }
    const Vec3d vertex(const int, const int) const;
    const Vec3d texvertex(const int, const int) const;
    const Vec3d normal(const int, const int) const;
    const Vec3i face(const int) const;

    size_t num_vertices() const { return verts_.size(); }
    size_t num_faces() const { return faces_.size(); }
    size_t num_normals() const { return norms_.size(); }
    size_t num_texvertices() const { return texverts_.size(); }

private:
    std::vector<Vec3d> verts_;
    std::vector<Vec3d> norms_;
    std::vector<Vec3d> texverts_; // texture vertices
    std::vector<Facet> faces_;
};

/*
 * ------------------ Functions ------------------
 */
void triangle_ref(Vec<3, int>, Vec<3, int>, Vec<3, int>, double, double, double,
        std::vector<int>&, PPM_Image&);

#endif

