#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"

// read texture from a file and return texture id
GLuint texture_from_file(const std::string&, const std::string&);

class Model {
public:
    explicit Model() = default;
    explicit Model(const std::string&);

    void draw(const Shader&);

private:
    std::vector<Mesh> meshes_ {}; // model data
    std::string dir_ {};
    std::vector<Texture> textures_loaded_ {}; // keep track of loaded textures

    void load_model(const std::string&);
    void process_node(aiNode*, const aiScene*);
    Mesh process_mesh(aiMesh*, const aiScene*);
    std::vector<Texture> load_material_textures(aiMaterial*,
            const aiTextureType, const std::string&);
};

#endif /* MODEL_H */

