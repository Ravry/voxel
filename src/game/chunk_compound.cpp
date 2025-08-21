#include "game/chunk_compound.h"


namespace Voxel::Game {
    ChunkCompound::ChunkCompound(Noise& noise, glm::vec3 position) : position(position) {
        //HEIGHT-MAP-INIT
        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                 height_map[x + (z * SIZE)] = noise.fetch_heightmap(position.x + x, position.z + z);
            }
        }

        //TREE-POS-INIT
        std::vector<glm::ivec2> tree_positions {
            glm::ivec2(2 + rand() % (SIZE - 5), 2 + rand() % (SIZE - 5))
        };

        //SINGLE-CHUNK-GENERATION
        for (int i {0}; i < NUM_CHUNKS_PER_COMPOUND; i++) {
            std::shared_ptr<Chunk> chunk = Chunk::create(
                height_map,
                block_types,
                tree_positions,
                noise,
                glm::ivec3(position.x, i * SIZE, position.z)
            );
            if (!chunk->is_empty) chunks.push_back(chunk);
        }
    }

    void ChunkCompound::build_chunk_meshes() {
        //BUILD-SINGLE-CHUNKS
        for (auto& chunk : chunks) {
            chunk->build_mesh();
        }
    }

    void ChunkCompound::render(Plane* frustum, Shader& shader) {
        //RENDER-IN-FRUSTUM-SINGLE-CHUNKS
        for (const auto& chunk : chunks) {
            if (!is_box_in_frustum(frustum, chunk->position, chunk->position + glm::ivec3(SIZE)))
                continue;

            chunk->render(shader);
        }
    }

    void ChunkCompound::unload() {
        //UNLOADING
        // for (const auto& chunk : chunks) {
        //     chunk->unload();
        // }
    }
}
