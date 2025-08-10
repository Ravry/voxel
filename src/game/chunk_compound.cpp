#include "chunk_compound.h"
#include "gizmo.h"

namespace Voxel::Game {
    static std::unique_ptr<VAO> vao_box_gizmo;

    ChunkCompound::ChunkCompound(Noise& noise, glm::vec3 position) : position(position) {
        if (!vao_box_gizmo.get()) {
            vao_box_gizmo = std::make_unique<VAO>();
            Gizmo::setup_line_box_gizmo(*vao_box_gizmo.get());
        }


        int height_map[SIZE * SIZE];
        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                 height_map[x + (z * SIZE)] = 4 * SIZE + (int)(noise.fetch_heightmap(x + position.x, z + position.z) * 2 * SIZE);
            }
        }

        std::vector<glm::ivec2> tree_positions {
            // glm::ivec2(rand() % 16, rand() % 16)
            glm::ivec2(8, 8)
        };

        for (int i {0}; i < num_chunks_per_compound; i++) {
            std::shared_ptr<Chunk> chunk = Chunk::create(height_map, tree_positions, noise, glm::ivec3(position.x, i * SIZE, position.z));
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
            // Gizmo::render_line_box_gizmo(*vao_box_gizmo.get(), chunk->position, glm::vec3(16.f));
            chunk->render();
        }
    }
}