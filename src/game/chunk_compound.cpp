#include "chunk_compound.h"


namespace Voxel::Game {
    ChunkCompound::ChunkCompound(Noise& noise, glm::vec3 position) : position(position) {
        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                 height_map[x + (z * SIZE)] = noise.fetch_heightmap(position.x + x, position.z + z);
            }
        }

        std::vector<glm::ivec2> tree_positions {
            glm::ivec2(2 + rand() % (SIZE - 5), 2 + rand() % (SIZE - 5))
        };

        for (int i {0}; i < num_chunks_per_compound; i++) {
            Chunk* chunk = Chunk::create(height_map, block_types, tree_positions, noise, glm::ivec3(position.x, i * SIZE, position.z));
            if (!(chunk->is_empty))
                chunks.push_back(chunk);
        }
    }

    void ChunkCompound::build_chunk_meshes() {
        for (auto& chunk : chunks) {
            chunk->build_mesh();
        }
    }

    void ChunkCompound::render(Camera& camera) {
        for (const auto& chunk : chunks) {
            if (camera.is_box_in_frustum(camera.frustum, chunk->position, chunk->position + glm::ivec3(16.f)))
                chunk->render();
        }
    }

    void ChunkCompound::unload() {
        for (const auto& chunk : chunks) {
            chunk->unload();
        }
    }

}
