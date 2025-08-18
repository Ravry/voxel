#include "transform.h"

namespace Voxel {
    Transform::Transform() {
        matrix = glm::mat4(1);
    }

    Transform::Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
        matrix = glm::scale(
            glm::translate(
                glm::mat4(1.f),
                translation
            ),
            scale
        );
    }
}