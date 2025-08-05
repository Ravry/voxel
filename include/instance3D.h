#pragma once
#include <memory>
#include <vector>
#include <map>
#include <bit>
#include "transform.h"
#include "buffer.h"
#include "geometry.h"
#include "shader.h"

namespace Voxel::Game {
    class Chunk;
}

namespace Voxel {
    enum PrimitiveType {
        Triangle,
        Cube
    };  

    class Mesh {
    public:
        Mesh(PrimitiveType primitive);
        Mesh(uint16_t* voxels, Game::Chunk** chunk, size_t size);
        void render();
    private:
        VAO vao;
        VBO vbo;
        EBO ebo;
        unsigned int triangles {0};
    };

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
