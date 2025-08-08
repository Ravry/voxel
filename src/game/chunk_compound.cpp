#include "chunk_compound.h"
#include "gizmo.h"

namespace Voxel::Game {

    static std::unique_ptr<VAO> vao_box_gizmo;

    ChunkCompound::ChunkCompound(Noise& noise, glm::vec3 position) : position(position) {
        if (!vao_box_gizmo.get()) {
            vao_box_gizmo = std::make_unique<VAO>();
            Gizmo::setup_line_box_gizmo(*vao_box_gizmo.get());
        }

        for (int i {0}; i < 1; i++) {
            std::shared_ptr<Chunk> chunk = Chunk::create(noise, glm::vec3(position.x, i * 16, position.z));
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
            Gizmo::render_line_box_gizmo(*vao_box_gizmo.get(), chunk->position, glm::vec3(16.f));
            chunk->render();
        }
    }
}