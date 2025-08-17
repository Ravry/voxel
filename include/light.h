#pragma once
#include <glm/glm.hpp>

namespace Voxel {
    struct DirectionalLight {
        glm::vec3 direction;

        DirectionalLight(float rotation_x, float rotation_y, float rotation_z) {
            glm::quat q = glm::quat(glm::vec3(glm::radians(rotation_x), glm::radians(rotation_y), glm::radians(rotation_z)));
            direction = q * glm::vec3(0.0f, 0.0f, -1.0f);
            direction = glm::normalize(direction);
        }
    };
}