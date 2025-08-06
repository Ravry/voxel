#pragma once
#include "mesh.h"
#include "transform.h"
#include "shader.h"


namespace Voxel {
    class Instance3D : public Transform {
    public:
        Instance3D() = default;
        Instance3D(Mesh* model, glm::vec3 albedo, glm::vec3 position, glm::vec3 scale = glm::vec3(1));
        ~Instance3D();
        void render(Shader& shader);
    private:
        Mesh* model;
        glm::vec3 albedo;
    };
}
