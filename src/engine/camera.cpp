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
        get_frustum(frustum);
    }

    void Camera::refactor(float width, float height) {
        projection = glm::perspective(glm::radians(60.f), width/height, .01f, 1000.f);
    }

    void Camera::get_frustum(Plane* frustum) {
        glm::mat4 clip = projection * matrix;

        auto normalizePlane = [](Plane &plane) {
            float mag = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
            plane.a /= mag;
            plane.b /= mag;
            plane.c /= mag;
            plane.d /= mag;
        };

        // Left plane
        frustum[0].a = clip[0][3] + clip[0][0];
        frustum[0].b = clip[1][3] + clip[1][0];
        frustum[0].c = clip[2][3] + clip[2][0];
        frustum[0].d = clip[3][3] + clip[3][0];
        normalizePlane(frustum[0]);

        // Right plane
        frustum[1].a = clip[0][3] - clip[0][0];
        frustum[1].b = clip[1][3] - clip[1][0];
        frustum[1].c = clip[2][3] - clip[2][0];
        frustum[1].d = clip[3][3] - clip[3][0];
        normalizePlane(frustum[1]);

        // Bottom plane
        frustum[2].a = clip[0][3] + clip[0][1];
        frustum[2].b = clip[1][3] + clip[1][1];
        frustum[2].c = clip[2][3] + clip[2][1];
        frustum[2].d = clip[3][3] + clip[3][1];
        normalizePlane(frustum[2]);

        // Top plane
        frustum[3].a = clip[0][3] - clip[0][1];
        frustum[3].b = clip[1][3] - clip[1][1];
        frustum[3].c = clip[2][3] - clip[2][1];
        frustum[3].d = clip[3][3] - clip[3][1];
        normalizePlane(frustum[3]);

        // Near plane
        frustum[4].a = clip[0][3] + clip[0][2];
        frustum[4].b = clip[1][3] + clip[1][2];
        frustum[4].c = clip[2][3] + clip[2][2];
        frustum[4].d = clip[3][3] + clip[3][2];
        normalizePlane(frustum[4]);

        // Far plane
        frustum[5].a = clip[0][3] - clip[0][2];
        frustum[5].b = clip[1][3] - clip[1][2];
        frustum[5].c = clip[2][3] - clip[2][2];
        frustum[5].d = clip[3][3] - clip[3][2];
        normalizePlane(frustum[5]);
    }

    bool Camera::is_box_in_frustum(const Plane planes[6], const glm::vec3& min, const glm::vec3& max) {
        for (int i = 0; i < 6; i++) {
            const Plane& p = planes[i];

            // Select the "positive vertex" in direction of plane normal
            glm::vec3 positive = min;
            if (p.a >= 0) positive.x = max.x;
            if (p.b >= 0) positive.y = max.y;
            if (p.c >= 0) positive.z = max.z;

            // Distance from plane
            float distance = p.a * positive.x + p.b * positive.y + p.c * positive.z + p.d;

            if (distance < 0) {
                // Outside the frustum
                return false;
            }
        }
        return true; // Inside or intersecting
    }
}
