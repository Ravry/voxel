#pragma once
#include <algorithm>
#include <GLFW/glfw3.h>
#include "transform.h"
#include "input.h"

namespace Voxel {
    class Camera : public Transform {
        private:
            float yaw {-90.f};
            float pitch {0.f};
            glm::mat4 projection;
        public:
            glm::vec3 position {glm::vec3(0, 0, 2)};
            glm::vec3 front;

        public:
            Camera(float width, float height);
            void update(GLFWwindow* window, float delta_time);
            void refactor(float width, float height);

            glm::mat4 get_projection() {
                return projection;
            }
    };
}