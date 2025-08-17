#include "model.h"
#include "log.h"

namespace Voxel {
    std::unique_ptr<Mesh> Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        for (size_t i {0}; i < mesh->mNumVertices; i++) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].x);

            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        for (size_t i {0}; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (size_t j {0}; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        if(mesh->mMaterialIndex >= 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            for (size_t i {0}; i < aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE); i++) {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, i, &str);

                Texture::TextureCreateInfo texture_create_info { GL_TEXTURE_2D };
                texture_create_info.type = GL_UNSIGNED_BYTE;
                texture_create_info.format = GL_RGBA;
                texture_create_info.internal_format = GL_RGBA;
                texture_create_info.wrap = GL_CLAMP_TO_EDGE;
                texture_create_info.min_filter = GL_NEAREST;
                texture_create_info.mag_filter = GL_NEAREST;
                texture_create_info.file_path = std::format("{}/{}", model_path, str.C_Str()).c_str();
                LOG("loading model-texture: {}", texture_create_info.file_path);
                texture = std::make_unique<Texture>(texture_create_info);
            }
        }
        return std::make_unique<Mesh>(vertices, indices);
    }

    void Model::process_node(aiNode* node, const aiScene* scene) {
        for (size_t i {0}; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(process_mesh(mesh, scene));
        }

        for (size_t i {0}; i < node->mNumChildren; i++) {
            process_node(node->mChildren[i], scene);
        }
    }

    Model::Model(std::string_view path) : model_path(path.substr(0, path.rfind('/'))) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path.data(),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PreTransformVertices
        );

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            LOG("error loading model: {}", importer.GetErrorString());
            return;
        }

        process_node(scene->mRootNode, scene);
    }
}
