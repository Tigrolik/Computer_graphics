/*
 * Mesh - class used for meshes in Assimp
 *
 */

#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <GL/glew.h>      // OpenGL lib
#include <glm/glm.hpp>    // vector mathematics
#include <assimp/scene.h> // assimp (for aiString)
#include "Shader.h"

// Vertex structure: vectors for positions, normals and texture coordinates
struct Vertex {
    glm::vec3 position {};
    glm::vec3 normal {};
    glm::vec2 tex_coords {};
};

// Texture structure: id and type of a texture
struct Texture {
    GLuint id {};
    std::string type {};
    aiString path {};
};

// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_of_elements(const std::vector<T> &v) {
    return v.size() * sizeof(T);
}

// Mesh class
class Mesh {
public:
    explicit Mesh(const std::vector<Vertex>&, const std::vector<GLuint>&,
            const std::vector<Texture>&);

    void draw(const Shader&);

private:
    std::vector<Vertex> vertices_;
    std::vector<GLuint> indices_;
    std::vector<Texture> textures_;
    GLuint VAO_, VBO_, EBO_; // data for rendering

    void setup();
};

#endif /* MESH_H */

