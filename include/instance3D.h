#pragma once
#include "mesh.h"
#include "transform.h"
#include "shader.h"
#include "buffer.h"


namespace Voxel {
    class Instance3D : public Transform {
    public:
        Instance3D() = default;
        Instance3D(Mesh* mesh, glm::vec3 albedo, glm::vec3 position, glm::vec3 scale = glm::vec3(1));
        ~Instance3D();
        void render(Shader& shader);
    private:
        Mesh* mesh;
        glm::vec3 albedo;
        VAO vao;
        VBO vbo;
        EBO ebo;
    };
}
