#include "chunk_manager.h"


namespace Voxel::Game {
    static int64_t chunk_position_to_key(int x, int z) {
        return ((int64_t)x << 32) | (uint32_t)z;
    }

    static glm::vec3 chunk_key_to_position(int64_t key) {
        return glm::vec3(key >> 32, 0, (int32_t)key);
    }

    ChunkManager::ChunkManager(glm::ivec3 position) {
        on_new_chunk_entered(position);
    }

    void ChunkManager::update(glm::ivec3 world_space_position) {
        glm::ivec3 chunk_space_position = glm::ivec3(
            (world_space_position.x / SIZE) * SIZE,
            0,
            (world_space_position.z / SIZE) * SIZE
        );

        static glm::ivec3 old_position = chunk_space_position;
        if (old_position != chunk_space_position) {
            old_position = chunk_space_position;
            on_new_chunk_entered(chunk_space_position);
        }
    }

    static std::unordered_map<int64_t, std::unique_ptr<ChunkCompound>> rendered_chunks;
    static std::unordered_map<int64_t, std::unique_ptr<ChunkCompound>> cached_chunks;

    void ChunkManager::render_chunk_compounds() {
        for (auto& chunk : rendered_chunks) {
            chunk.second->render();
        }
    }

    void ChunkManager::on_new_chunk_entered(glm::ivec3 chunk_space_position) {
        // std::cout << "position updated!\n   position: " << chunk_space_position.x << "; " << chunk_space_position.z << std::endl;

        const int render_distance_squared = chunk_render_distance * chunk_render_distance;
        std::unordered_set<uint64_t> needed_chunks;
        for (int x = -chunk_render_distance; x <= chunk_render_distance; x++) {
            for (int z = -chunk_render_distance; z <= chunk_render_distance; z++) {
                if (x * x + z * z <= render_distance_squared) {
                    needed_chunks.insert(chunk_position_to_key(chunk_space_position.x + x * SIZE, chunk_space_position.z + z * SIZE));
                }
            }
        }

        for (auto chunk : needed_chunks) {
            if (rendered_chunks.contains(chunk)) continue;

            if (cached_chunks.find(chunk) == cached_chunks.end()) {
                rendered_chunks[chunk] = std::make_unique<ChunkCompound>(noise, chunk_key_to_position(chunk));
            }
            else {
                rendered_chunks[chunk] = std::move(cached_chunks[chunk]);
                cached_chunks.erase(chunk);
            }
        }

        std::vector<uint64_t> chunks_to_remove;
        for (auto& chunk : rendered_chunks) {
            if (!needed_chunks.contains(chunk.first)) {
                cached_chunks[chunk.first] = std::move(chunk.second);
                chunks_to_remove.push_back(chunk.first);
            }
            else
                chunk.second->build_chunk_meshes();
        }

        for (auto chunk : chunks_to_remove) {
            rendered_chunks.erase(chunk);
        }
    }
}
