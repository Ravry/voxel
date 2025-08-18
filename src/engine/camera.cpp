#include "camera.h"

#include "log.h"

namespace Voxel {
    static bool cursor_enabled = false;

    Camera::Camera(float width, float height, glm::vec3 position) : Transform(glm::vec3(0), glm::vec3(0), glm::vec3(1)), position(position) {
        projection = glm::perspective(glm::radians(60.f), width/height, .1f, 1000.f);
    }

    void Camera::update(GLFWwindow* window, float delta_time) {
        if (Input::is_key_pressed(GLFW_KEY_ESCAPE)) {
            cursor_enabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (Input::is_key_pressed(GLFW_MOUSE_BUTTON_2)) {
            cursor_enabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

        matrix = glm::translate(matrix, input);

        speed_multiplier = Input::is_key_held_down(GLFW_KEY_LEFT_SHIFT) ? 4.f : 1.f;

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

        glm::vec3 move = (input.z * cameraFront + input.y * cameraUp + input.x * cameraRight) * ((float)delta_time * speed * speed_multiplier);
        position += move; 

        matrix = glm::lookAt(position, position + cameraFront, cameraUp);
        get_frustum(frustum, projection, matrix);
    }

    void Camera::refactor(float width, float height) {
        projection = glm::perspective(glm::radians(60.f), width/height, .01f, 1000.f);
    }
}
