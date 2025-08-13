#pragma once
#include <glm/glm.hpp>
#include "chunk.h"

namespace Voxel::Game {
    const int num_chunks_per_compound = 16;

    class ChunkCompound {
    public:
        ChunkCompound(Noise& noise, glm::vec3 position);
        void build_chunk_meshes();
        void render();
        void unload();

        glm::vec3 position;
    private:
        int height_map[SIZE * SIZE];
        BlockType block_types[SIZE * (SIZE * num_chunks_per_compound) * SIZE] {};
        std::vector<Chunk*> chunks;

    };
}