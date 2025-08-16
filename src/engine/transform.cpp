#include "transform.h"

namespace Voxel {
    Transform::Transform() {
        matrix = glm::mat4(1);
    }

    Transform::Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
        matrix = glm::scale(glm::translate(glm::mat4(1.f), translation), scale);
        // matrix = glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0, 40.f, -127.f)), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    }
}