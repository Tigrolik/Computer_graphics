/*
 * Model reads an .obj file and stores the data in vectors
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include <fstream>
#include <vector>

class Model {
public:
    Model(const std::string&);

    ~Model() = default;

    std::vector<double> vertex(int i) const { return verts_[i]; };
    std::vector<int> face(int i) const { return faces_[i]; };
    int num_vertices() const { return verts_.size(); }
    int num_faces() const { return faces_.size(); }

private:
    std::vector<std::vector<double>> verts_;
    std::vector<std::vector<int>> faces_;
};

#endif

