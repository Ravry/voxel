#pragma once
#include <glm/glm.hpp>
#include "chunk.h"
#include "engine/camera.h"

namespace Voxel::Game {
    class ChunkCompound {
    public:
        ChunkCompound(Noise& noise, glm::vec3 position);
        void build_chunk_meshes();
        void render(Plane* frustum, Shader& shader);
        void unload();

        glm::vec3 position;

    private:
        int height_map[SIZE * SIZE];
        unsigned int block_types[SIZE * (SIZE * NUM_CHUNKS_PER_COMPOUND) * SIZE] {};
        std::vector<std::shared_ptr<Chunk>> chunks;
    };
}