#pragma once
#include <memory>
#include <unordered_map>
#include "glm/glm.hpp"
#include "instance3D.h"
#include "noise.h"
#include "misc.h"
#include "resource_manager.h"

namespace Voxel {
    namespace Game {
        const int SIZE = sizeof(uint16_t) * 8;

        class Chunk {
        public:
            static std::shared_ptr<Chunk> create(int* height_map, BlockType* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position);

            Chunk() = default;
            Chunk(int* height_map, BlockType* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position);
            void build_mesh();
            void render();

            glm::ivec3 position;

            std::shared_ptr<Mesh> mesh;
            std::shared_ptr<Instance3D> instance;
            std::shared_ptr<SSBO> ssbo;
            bool is_empty {true};
            uint16_t voxels[SIZE * SIZE * 3] = {};
        private:
            void generate_trees(int* height_map, std::vector<glm::ivec2>& tree_positions);
            BlockType* block_types_ptr {nullptr};
            bool built {false};
        };
    }
}