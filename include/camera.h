#pragma once
#include <algorithm>
#include <GLFW/glfw3.h>
#include "transform.h"
#include "input.h"

namespace Voxel {
    class Camera : public Transform {

        enum CameraMode {
            FirstPerson,
            FreeCam,
        };

        private:
            float yaw {90.f};
            float pitch {0.f};
            glm::mat4 projection;
            float speed {8.f};
            float speed_multiplier {1.f};

        public:
            glm::vec3 position;
            glm::vec3 front;
        public:
            Camera(float width, float height, glm::vec3 position);
            void update(GLFWwindow* window, float delta_time);
            void refactor(float width, float height);

            struct Plane {
                float a, b, c, d; // Plane equation: ax + by + cz + d = 0
            };
            void get_frustum(Plane* planes);
            bool is_box_in_frustum(const Plane planes[6], const glm::vec3& min, const glm::vec3& max);

            glm::mat4 get_projection() { return projection; }
    };
}