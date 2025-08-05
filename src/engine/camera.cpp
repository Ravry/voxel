#include "camera.h"

namespace Voxel {
    static bool cursor_enabled = false;

    Camera::Camera(float width, float height) : Transform(glm::vec3(0), glm::vec3(0), glm::vec3(1)) {
        projection = glm::perspective(glm::radians(60.f), width/height, .01f, 100.f);
    }

    void Camera::update(GLFWwindow* window, float delta_time) {
        if (Input::is_key_pressed(GLFW_KEY_ESCAPE)) {
            cursor_enabled = !cursor_enabled;
            glfwSetInputMode(window, GLFW_CURSOR, cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }

        if (cursor_enabled)
            return;

        glm::vec3 input = glm::vec3(0.f);
        if (glfwGetKey(window, GLFW_KEY_W)) {
            input.z += 1; 
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            input.z -= 1; 
        }
        
        if (glfwGetKey(window, GLFW_KEY_A)) {
            input.x -= 1; 
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            input.x += 1; 
        }
        
        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            input.y += 1; 
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
            input.y -= 1; 
        }

        if (glm::length(input) > 0) {
            input = glm::normalize(input);
        }

        yaw += Input::delta_x;
        pitch -= Input::delta_y;
        pitch = std::clamp(pitch, -80.f, 80.f);

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 cameraFront = glm::normalize(front);
        glm::vec3 cameraUp = glm::vec3(0, 1, 0);
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

        glm::vec3 move = (input.z * cameraFront + input.y * cameraUp + input.x * cameraRight) * (float)delta_time * 8.f;
        position += move; 

        matrix = glm::lookAt(position, position + cameraFront, cameraUp);
    }

    void Camera::refactor(float width, float height) {
        projection = glm::perspective(glm::radians(60.f), width/height, .01f, 100.f);
    }
}