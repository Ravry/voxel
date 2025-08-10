#pragma once
#include <bit>
#include <vector>
#include <string_view>
#include "buffer.h"
#include "geometry.h"

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
        Mesh(uint16_t* voxels, Game::Chunk** chunk, const size_t size);
        Mesh(std::string_view filename);
        void render();
    private:
        VAO vao;
        VBO vbo;
        EBO ebo;
        unsigned int triangles {0};
    };
}