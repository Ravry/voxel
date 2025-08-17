#pragma once
#include <bit>
#include <vector>
#include <stdint.h>
#include <string_view>
#include "geometry.h"

namespace Voxel::Game {
    class Chunk;
}

namespace Voxel {
    enum PrimitiveType {
        Triangle,
        Quad,
        Cube
    };

    class Mesh {
    public:
        Mesh(PrimitiveType primitive);
        Mesh(uint16_t* voxels, Game::Chunk** chunk, const size_t size);
        Mesh(std::string_view filename);
        Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices);

        std::vector<uint32_t> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> vertices_float;
        unsigned int triangles {0};
    };
}