#include <stdexcept>
#include "Model.h"

Model::Model(const std::string &fn): verts_{}, faces_{} {
    std::ifstream ifs {fn};
    if (!ifs)
        throw std::runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);

    for (std::string s; ifs >> s;) {
        if (s == "v") {
            // read x, y, z coordinates of the vertices
            std::vector<double> v(3);
            for (auto &x: v)
                ifs >> x;
            verts_.push_back(v);
        }
        if (s == "f") {
            // reading the first numbers of triples: triangle vertices
            std::vector<int> v(3);
            for (auto &x: v) {
                ifs >> x; --x;
                char c; int i;
                ifs >> c >> i >> c >> i; // skip two slashes and two values
            }
            faces_.push_back(v);
        }
        if (ifs.fail()) {
            ifs.unget();
            ifs.clear(std::ios_base::failbit);
        }
    }
}

