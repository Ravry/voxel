#pragma once
#include <memory>
#include <map>
#include "glm/glm.hpp"
#include "instance3D.h"
#include "noise.h"
#include "misc.h"

namespace Voxel {
    namespace Game {
        const size_t SIZE = sizeof(uint16_t) * 8;

        class Chunk {
        public:
            static std::shared_ptr<Chunk> create(Noise& noise, glm::vec3 position);

            Chunk() = default;
            Chunk(Noise& noise, glm::vec3 position);
            void build_mesh();
            void render();

            glm::vec3 position;
            uint16_t voxels[SIZE * SIZE * 3] = {};
        private:
            unsigned int data[SIZE * SIZE * SIZE];
            std::shared_ptr<Mesh> mesh;
            std::shared_ptr<Instance3D> instance;

            bool built {false};
        };
    }
}