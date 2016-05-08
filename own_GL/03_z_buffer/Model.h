/*
 * Model reads an .obj file and stores the data in vectors
 * In this exercise we introduce vector operations (sum, difference, dot and
 * vector products etc.) implemented as overloaded operators and functions.
 * However, in the main code we use functions and not operators
 *
 * This header also contains function for drawing rendered images (z buffer and
 * applying texture on the model)
 * The codes look rather messy, they shall be improved in the next exercises
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include <fstream>
#include <vector>
#include "PPM_Image.h"
#include "Geometry.h"

using namespace std;

template <class T>
vector<T> operator+(const vector<T> &v1, const vector<T> &v2) {
    const auto n = v1.size();
    vector<T> w(n);
    if (n != v2.size())
        throw runtime_error("error: vectors sizes do not match");
    else
        for (size_t i {0}; i < n; ++i)
            w[i] = v1[i] + v2[i];
    return w;
}

template <class T>
vector<T> operator-(const vector<T> &v1, const vector<T> &v2) {
    const auto n = v1.size();
    vector<T> w(n);
    if (n != v2.size())
        throw runtime_error("error: vectors sizes do not match");
    else
        for (size_t i {0}; i < n; ++i)
            w[i] = v1[i] - v2[i];
    return w;
}

template <class T>
vector<double> operator*(const vector<T> &v, const double d) {
    const auto n = v.size();
    vector<double> w(n);
    for (size_t i {0}; i < n; ++i)
        w[i] = v[i] * d;
    return w;
}

template <class T>
T operator*(const vector<T> &v1, const vector<T> &v2) {
    return inner_product(v1.begin(), v1.end(), v2.begin(), T{});
}

template <class T>
vector<T> operator^(const vector<T> &v1, const vector<T> &v2) {
    const auto n = v1.size();
    if (n != v2.size() || n != 3) {
        throw runtime_error("error: vectors sizes do not match");
        return vector<T> {};
    }
    return vector<T> {
        v1[1] * v2[2] - v1[2] * v2[1], // y * z - z * y
            v1[2] * v2[0] - v1[0] * v2[2], // z * x - x * z
            v1[0] * v2[1] - v1[1] * v2[0], // x * y - y * x
    };
}

class Vertex {
public:
    Vertex(const double, const double, const double);

    ~Vertex() = default;

    double x() const { return v_[0]; }
    double y() const { return v_[1]; }
    double z() const { return v_[2]; }
    vector<double> values() const { return v_; }

private:
    vector<double> v_{0, 0, 0};
};

class Facet {
public:
private:
    vector<int> v_;
};

class Model {
public:
    Model(const string&);

    ~Model() = default;

    vector<double> vertex(const int i) const { return verts_[i]; };
    vector<int> face(const int) const;
    vector<double> normal(const int i) const { return norms_[i]; };
    vector<double> texvertex(const int, const int) const;

    int num_vertices() const { return verts_.size(); }
    int num_faces() const { return faces_.size(); }
    int num_normals() const { return norms_.size(); }
    int num_texvertices() const { return texverts_.size(); }

private:
    vector<vector<double>> verts_;
    vector<vector<vector<int>>> faces_;
    vector<vector<double>> norms_;
    vector<vector<double>> texverts_;
};

// not implementing templates yet
vector<double> sum(const vector<double>&, const vector<double>&);

vector<double> diff(const vector<double>&, const vector<double>&);

vector<double> prod(const vector<double>&, const double);

double dot(const vector<double>&, const vector<double>&);

vector<double> cross(const vector<double>&, const vector<double>&);

double norm(const vector<double>&);

void normalize(vector<double>&, const double p = 1);

vector<int> round(const vector<double>&);

vector<double> itod(const vector<int>&);

void rasterize(Point&, Point&, vector<int>&, PPM_Image&,
        const PPM_Color& = Color_name::white);

void triangle_buf(vector<int>, vector<int>, vector<int>,
        vector<int>&, PPM_Image&, const PPM_Color& = Color_name::white);

void triangle_tex(vector<int>, vector<int>, vector<int>, vector<int>,
        vector<int>, vector<int>, vector<int>&, PPM_Image&, PPM_Image&,
        const double = 1.0);

#endif

