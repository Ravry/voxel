#pragma once
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "engine/resource_manager.h"
#include "engine/buffer.h"
#include "engine/mesh.h"
#include "engine/buffer_allocator.h"
#include "engine/gizmo.h"
#include "engine/physics_manager.h"
#include "game/noise.h"
#include "game/misc.h"

namespace Voxel {
    namespace Game {
        class Chunk {
        public:
            static std::shared_ptr<Chunk> create(int* height_map, unsigned int* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position);

            Chunk() = default;
            Chunk(int* height_map, unsigned int* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position);
            void build_mesh();
            void render(Shader& shader);

            void load();
            void unload();

        private:
            void set_block_type(int index, uint8_t block);
            void set_block_type(int x, int y, int z, uint8_t block);
            uint8_t access_block_type(int x, int y, int z);
            void set_block(int x, int y, int z, uint8_t block);
            void generate_trees(Noise& noise, int* height_map, std::vector<glm::ivec2>& tree_positions);
            bool find_neighbours(std::vector<uint16_t*>& neighbours);
        public:
            glm::ivec3 position;
            std::unique_ptr<Mesh<uint32_t>> mesh;
            std::unique_ptr<SSBO> ssbo;

            uint16_t voxels[SIZE * SIZE * 3] = {};
            unsigned int* block_types_ptr {nullptr};

            bool is_empty {true};
            bool built {false};
            bool allocated {false};
            unsigned int slot {0};
            unsigned int slot_physics {0};

            JPH::Ref<JPH::Shape> shape;

        };
    }
}