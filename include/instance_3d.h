#pragma once
#include <memory>
#include "mesh.h"
#include "buffer.h"
#include "material.h"

namespace Voxel {
    class Instance3D : public Transform {
    private:
        std::unique_ptr<VAO> vao;
        std::unique_ptr<VBO> vbo;
        std::unique_ptr<EBO> ebo;
        Material* material;
        Mesh<float>* mesh;
    public:
        Instance3D(Mesh<float>* mesh, const VAO::AttribInfo& attrib_info, Material* material, glm::vec3 translation);
        ~Instance3D() = default;
        void render();
    };
}