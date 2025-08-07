#pragma once
#include <unordered_map>
#include <queue>
#include "chunk_compound.h"

namespace Voxel::Game {
    class ChunkManager {
    public:
        ChunkManager(glm::ivec3 position);
        void update(glm::ivec3 position);

        ChunkCompound* get_chunk_coumpound(glm::vec2 position) {
            auto it = cached_chunk_compounds.find(position.x + position.y * 16);
            return (it != cached_chunk_compounds.end()) ? it->second.get() : nullptr;
        };

        void generate_chunk_compounds(glm::ivec2 position);
        void render_chunk_compounds();
    private:
        Noise noise;

        std::queue<std::unique_ptr<ChunkCompound>> chunk_compounds_to_load;
        std::unordered_map<int, std::unique_ptr<ChunkCompound>> cached_chunk_compounds;
        std::vector<ChunkCompound*> rendered_chunk_compounds;

        int chunk_render_distance {5};
    };
}