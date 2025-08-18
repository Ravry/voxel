#pragma once
#include <glm/glm.hpp>
#include "camera.h"

namespace Voxel {
    constexpr unsigned int SHADOW_MAP_SIZE {4096};
    constexpr float ortho_size {50.f};

    struct DirectionalLight {
        Plane frustum[6];
        glm::vec3 direction;

        DirectionalLight(float rotation_x, float rotation_y, float rotation_z) {
            glm::quat q = glm::quat(glm::vec3(glm::radians(rotation_x), glm::radians(rotation_y), glm::radians(rotation_z)));
            direction = q * glm::vec3(0.0f, 0.0f, -1.0f);
            direction = glm::normalize(direction);
        }

        void update(Camera* camera) {
            shadow_map_projection_matrix = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, 1.0f, 500.f);
            shadow_map_view_matrix = glm::lookAt(
                camera->position - direction * 120.f,
                camera->position,
                glm::normalize(glm::cross(direction, glm::vec3(1.f, 0.f, 0.f)))
            );

            get_frustum(frustum, shadow_map_projection_matrix, shadow_map_view_matrix);
        }

        glm::mat4 get_light_space_matrix() {
            return shadow_map_projection_matrix * shadow_map_view_matrix;
        }

        glm::mat4 shadow_map_projection_matrix;
        glm::mat4 shadow_map_view_matrix;
    private:
    };
}