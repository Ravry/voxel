#pragma once
#include <bit>
#include <vector>
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
        Mesh(uint16_t* voxels, Game::Chunk** chunk, size_t size);
        void render();
    private:
        VAO vao;
        VBO vbo;
        EBO ebo;
        unsigned int triangles {0};
    };
}