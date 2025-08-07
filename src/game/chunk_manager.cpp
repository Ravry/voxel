#include "chunk_manager.h"

namespace Voxel::Game {

    ChunkManager::ChunkManager(glm::ivec3 position) {
        generate_chunk_compounds(glm::vec2(position.x, position.z));

        for (auto& chunk_compound : cached_chunk_compounds) {
            chunk_compound.second->build_chunk_meshes();
        }
    }

    void ChunkManager::update(glm::ivec3 position) {
        static glm::ivec3 old_position = position;
        if (old_position != position) {
            std::cout << "position updated" << std::endl;
            old_position = position;
            rendered_chunk_compounds.clear();
            generate_chunk_compounds(glm::vec2(position.x, position.z));
        }
    }

    void ChunkManager::render_chunk_compounds() {
        for (auto chunk_compound : rendered_chunk_compounds) {
            chunk_compound->render();
        }
    }

    void ChunkManager::generate_chunk_compounds(glm::ivec2 position) {
        for (int i {0}; i < chunk_render_distance * chunk_render_distance; i++) {
            int x = ((i % chunk_render_distance) * SIZE) - (chunk_render_distance * SIZE / 2);
            x += position.x;
            int z = (((i / chunk_render_distance) % chunk_render_distance) * SIZE) - (chunk_render_distance * SIZE / 2);
            z += position.y;

            ChunkCompound* chunk_compound = get_chunk_coumpound(glm::vec2(x, z));
            if (chunk_compound != nullptr) {
                rendered_chunk_compounds.push_back(chunk_compound);
            } else {
                auto new_compound = std::make_unique<ChunkCompound>(noise, glm::vec2(x, z));
                new_compound->build_chunk_meshes();
                cached_chunk_compounds[x + z * SIZE] = std::move(new_compound);
            }
        }
    }
}