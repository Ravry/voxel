#include "chunk_compound.h"

namespace Voxel::Game {

    ChunkCompound::ChunkCompound(Noise& noise, glm::vec3 position) : position(position) {
        int height_map[SIZE * SIZE];
        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                 height_map[x + (z * SIZE)] = noise.fetch_heightmap(position.x + x, position.z + z);
            }
        }

        std::vector<glm::ivec2> tree_positions {
            // glm::ivec2(rand() % 16, rand() % 16)
            glm::ivec2(8, 8)
        };

        for (int i {0}; i < num_chunks_per_compound; i++) {
            std::shared_ptr<Chunk> chunk = Chunk::create(height_map, tree_positions, noise, glm::ivec3(position.x, i * SIZE, position.z));
            if (!chunk->is_empty)
                chunks.push_back(std::move(chunk));
        }
    }

    void ChunkCompound::build_chunk_meshes() {
        for (auto& chunk : chunks) {
            chunk->build_mesh();
        }
    }

    void ChunkCompound::render() {
        for (const auto& chunk : chunks) {
            chunk->render();
        }
    }
}