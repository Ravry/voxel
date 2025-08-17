#pragma once
#include <string_view>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "model.h"
#include "mesh.h"
#include "texture.h"

namespace Voxel {
    class Model {
    private:
        std::string_view model_path;
        void process_node(aiNode* node, const aiScene* scene);
        std::unique_ptr<Mesh> process_mesh(aiMesh *mesh, const aiScene *scene);
    public:
        std::vector<std::unique_ptr<Mesh>> meshes;
        std::unique_ptr<Texture> texture;
        Model(std::string_view path);
    };
}