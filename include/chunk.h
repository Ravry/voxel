#pragma once
#include <memory>
#include <unordered_map>
#include "glm/glm.hpp"
#include "noise.h"
#include "misc.h"
#include "resource_manager.h"
#include "buffer.h"
#include "mesh.h"

namespace Voxel {
    namespace Game {
        class Chunk {
        public:
            static Chunk* create(int* height_map, BlockType* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position);

            Chunk() = default;
            Chunk(int* height_map, BlockType* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position);
            void build_mesh();
            void render(Shader& shader);
            void unload();

            glm::ivec3 position;
            std::unique_ptr<Mesh<uint32_t>> mesh;
            std::unique_ptr<SSBO> ssbo;
            bool is_empty {true};
            uint16_t voxels[SIZE * SIZE * 3] = {};
        private:
            void set_block(int x, int y, int z, BlockType block);
            void generate_trees(Noise& noise, int* height_map, std::vector<glm::ivec2>& tree_positions);

            BlockType* block_types_ptr {nullptr};
            bool built {false};
            bool allocated {false};
            unsigned int slot {0};

            JPH::Ref<JPH::Shape> shape;
            JPH::BodyID body_id;
        };
    }
}