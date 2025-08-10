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

        glm::vec3 position;
    private:
        std::vector<std::shared_ptr<Chunk>> chunks;
    };
}