#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Voxel {
    class Transform {
        protected:
            glm::mat4 matrix;
        public:
            Transform();
            Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);
            glm::mat4& get_matrix() {
                return matrix;
            }
    };
}