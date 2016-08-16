#include "Model.h"
#include <iostream>
#include <SOIL/SOIL.h>          // image loading
#include <assimp/Importer.hpp>  // model importing
#include <assimp/postprocess.h> // postprocessing

// constructor
Model::Model(const std::string& path) {
    load_model(path);
}

// drawing
void Model::draw(const Shader& shad) {
    for(auto &m: meshes_)
        m.draw(shad);
}

// loading
void Model::load_model(const std::string& path) {
    Assimp::Importer imp;
    const aiScene* scene = imp.ReadFile(path, aiProcess_Triangulate |
            aiProcess_FlipUVs);

    if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << imp.GetErrorString() << '\n';
        return;
    }
    dir_ = path.substr(0, path.find_last_of('/'));
    process_node(scene->mRootNode, scene);
}

// node processing
void Model::process_node(aiNode* node, const aiScene* scene) {
    // process meshes
    for (GLuint i {0}; i < node->mNumMeshes; ++i) {
        aiMesh* m = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(process_mesh(m, scene));
    }
    // repeat for the children
    for (GLuint i {0}; i < node->mNumChildren; ++i)
        process_node(node->mChildren[i], scene);
}

// mesh processing
Mesh Model::process_mesh(aiMesh* mesh_p, const aiScene* scene) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    std::vector<Texture> texs;
    // vertices
    for (GLuint i {0}; i < mesh_p->mNumVertices; ++i) {
        Vertex vtx;
        vtx.position = glm::vec3{mesh_p->mVertices[i].x,
            mesh_p->mVertices[i].y, mesh_p->mVertices[i].z};
        vtx.normal = glm::vec3{mesh_p->mNormals[i].x,
            mesh_p->mNormals[i].y, mesh_p->mNormals[i].z};
        if (mesh_p->mTextureCoords[0]) {
            vtx.tex_coords = glm::vec2{mesh_p->mTextureCoords[0][i].x,
                mesh_p->mTextureCoords[0][i].y};
        } else {
            vtx.tex_coords = glm::vec2{0, 0};
        }
        // put the vertex into the array
        verts.push_back(vtx);
    }
    // indices
    for (GLuint i {0}; i < mesh_p->mNumFaces; ++i) {
        aiFace face = mesh_p->mFaces[i];
        for (GLuint j {0}; j < face.mNumIndices; ++j)
            inds.push_back(face.mIndices[j]);
    }
    // materials
    if (int(mesh_p->mMaterialIndex) >= 0) {
        aiMaterial* mater = scene->mMaterials[mesh_p->mMaterialIndex];
        const std::vector<Texture> diffuse_maps {load_material_textures(mater,
                aiTextureType_DIFFUSE, "texture_diffuse")};
        texs.insert(texs.end(), diffuse_maps.begin(), diffuse_maps.end());
        const std::vector<Texture> specular_maps {load_material_textures(mater,
                aiTextureType_SPECULAR, "texture_specular")};
        texs.insert(texs.end(), specular_maps.begin(), specular_maps.end());
    }

    return Mesh {verts, inds, texs};
}

// load and generate texture
std::vector<Texture> Model::load_material_textures(aiMaterial* mater,
        const aiTextureType tex_type, const std::string& type_name) {
    std::vector<Texture> texs;
    for (GLuint i {0}; i < mater->GetTextureCount(tex_type); ++i) {
        aiString str;
        mater->GetTexture(tex_type, i, &str);
        GLboolean skip {false};
        // check if the texture has already been loaded
        for (GLuint j {0}; j < textures_loaded_.size(); ++j)
            if (textures_loaded_[j].path == str) {
                texs.push_back(textures_loaded_[j]);
                skip = true;
                break;
            }
        if (!skip) {
            Texture t;
            t.id = texture_from_file(str.C_Str(), dir_);
            t.type = type_name;
            t.path = str;
            texs.push_back(t);
            textures_loaded_.push_back(t);
        }
    }
    return texs;
}

// read texture from a file and return texture id
GLuint texture_from_file(const std::string& path, const std::string& dir) {
    const std::string img_fn {dir + '/' + path};
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    int img_w, img_h;
    unsigned char *img = SOIL_load_image(img_fn.c_str(), &img_w, &img_h, 0,
            SOIL_LOAD_RGB);

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, GL_RGB,
            GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(img);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex_id;
}
