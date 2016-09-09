#include <iostream>
#include "Mesh.h"

// constructor
Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds,
        const std::vector<Texture>& texs):
    vertices_{verts}, indices_{inds}, textures_{texs}, VAO_{}, VBO_{}, EBO_{}
{
    setup();
}

// drawing method
void Mesh::draw(const Shader& shad) {
    GLuint diffuse_idx {0}, specular_idx {0}, reflect_idx {0};
    for (GLuint i {0}; i < textures_.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        const std::string tex_type {textures_[i].type};
        std::string uni_str {};
        if (tex_type == "texture_diffuse")
            uni_str = tex_type + std::to_string(++diffuse_idx);
        else if (tex_type == "texture_specular")
            uni_str = tex_type + std::to_string(++specular_idx);
        else if (tex_type == "texture_reflection")
            uni_str = tex_type + std::to_string(++reflect_idx);
        glUniform1f(glGetUniformLocation(shad.id(), uni_str.c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures_[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // set default shininess
    glUniform1f(glGetUniformLocation(shad.id(), "shininess"), 16);

    // drawing the mesh
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // set things back to defaults
    for (GLuint i {0}; i < textures_.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// setting up
void Mesh::setup() {
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, size_of_elements(vertices_), &vertices_[0],
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_of_elements(indices_),
            &indices_[0], GL_STATIC_DRAW);

    const auto vert_size = sizeof(Vertex);
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (GLvoid*)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size,
            (GLvoid*)(offsetof(Vertex, normal)));
    // Vertex texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vert_size,
            (GLvoid*)(offsetof(Vertex, tex_coords)));

    glBindVertexArray(0);
}

