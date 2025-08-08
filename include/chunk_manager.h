#pragma once
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include "chunk_compound.h"

namespace Voxel::Game {
    class ChunkManager {
    public:
        ChunkManager();
        void update(glm::ivec3 position);
        void render_chunk_compounds();
    private:
        void on_new_chunk_entered(glm::ivec3 chunk_space_position);
    private:
        Noise noise;
        int chunk_render_distance {4};
    };
}