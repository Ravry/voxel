#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "instance3D.h"
#include "noise.h"
#include "misc.h"

namespace Voxel {
    namespace Game {
        const size_t SIZE = sizeof(uint16_t) * 8;

        class Chunk {
        public:
            Chunk() = default;
            Chunk(Noise& noise, glm::vec3 position);
            void render(Shader& shader, SSBO& ssbo);
            glm::vec3 position;

        private:
            unsigned int data[SIZE * SIZE * SIZE];
            std::shared_ptr<Mesh> mesh;
            std::shared_ptr<Instance3D> instance;

        };
    }
}