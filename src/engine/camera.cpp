#include "core/window.h"
#include "engine/camera.h"
#include "core/log.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Constraints/FixedConstraint.h"

namespace Voxel {
    static bool cursor_enabled = false;

    static glm::vec3 physics_dst_pos;
    static glm::vec3 physics_src_pos;
    static Body* body;

    Camera::Camera(float width, float height, glm::vec3 position) : Transform(glm::vec3(0), glm::vec3(0), glm::vec3(1)), position(position) {
        projection = glm::perspective(glm::radians(60.f), width/height, .1f, 1000.f);

        unsigned int slot {0};
        BodyCreationSettings settings(new CapsuleShape(.5f, .4f), Vec3(position.x, position.y, position.z), Quat::sIdentity(), EMotionType::Dynamic, PhysicsLayers::MOVING);
        settings.mMotionQuality = EMotionQuality::LinearCast;
        body = Physics::PhysicsManager::get_instance().add_body(settings, slot);
    }

    void Camera::update(float delta_time) {
        input = glm::vec3(0.f);

        if (Input::is_key_pressed(GLFW_KEY_ESCAPE)) {
            cursor_enabled = true;
            Window::instance->set_cursor_mode(GLFW_CURSOR_NORMAL);
        }
        if (Input::is_key_pressed(GLFW_MOUSE_BUTTON_2)) {
            cursor_enabled = false;
            Window::instance->set_cursor_mode(GLFW_CURSOR_DISABLED);
        }

        if (cursor_enabled) return;

        if (Input::is_key_held_down(GLFW_KEY_W)) {
            input.z += 1; 
        }
        if (Input::is_key_held_down(GLFW_KEY_S)) {
            input.z -= 1; 
        }
        if (Input::is_key_held_down(GLFW_KEY_A)) {
            input.x -= 1; 
        }
        if (Input::is_key_held_down(GLFW_KEY_D)) {
            input.x += 1; 
        }
        if (Input::is_key_held_down(GLFW_KEY_SPACE)) {
            input.y += 1; 
        }
        if (Input::is_key_held_down(GLFW_KEY_LEFT_CONTROL)) {
            input.y -= 1; 
        }
        if (Input::is_key_held_down(GLFW_KEY_LEFT_SHIFT)) {speed_multiplier = 4.f;} else {speed_multiplier = 1.f;};

        if (glm::length(input) > 0) input = glm::normalize(input);

        yaw += Input::delta_x;
        pitch -= Input::delta_y;
        pitch = std::clamp(pitch, -80.f, 80.f);

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 cameraFront = glm::normalize(front);
        glm::vec3 cameraUp = glm::vec3(0, 1, 0);
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

        matrix = glm::lookAt(position, position + cameraFront, cameraUp);
        get_frustum(frustum, projection, matrix);

        glm::vec3 real_pos = glm::mix(physics_src_pos, physics_dst_pos, Physics::PhysicsManager::get_instance().lerp_t);
        position = real_pos;
    }

    void Camera::fixed_update() {
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 cameraFront = glm::normalize(front);
        glm::vec3 cameraUp = glm::vec3(0, 1, 0);
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

        glm::vec3 move = (input.z * cameraFront + input.y * cameraUp + input.x * cameraRight) * (speed * speed_multiplier);
        matrix = glm::lookAt(position, position + cameraFront, cameraUp);

        auto& body_interface = Physics::PhysicsManager::get_instance().m_implementation->physics_system.GetBodyInterface();
        auto velocity = body_interface.GetLinearVelocity(body->GetID());
        body_interface.SetLinearVelocity(body->GetID(), Vec3(move.x, velocity.GetY() + move.y, move.z));
        body_interface.SetAngularVelocity(body->GetID(), Vec3::sZero());
        body_interface.SetRotation(body->GetID(), Quat::sIdentity(), EActivation::DontActivate);
        physics_src_pos = position;
        auto body_pos = body_interface.GetPosition(body->GetID());
        physics_dst_pos = glm::vec3(body_pos.GetX(), body_pos.GetY(), body_pos.GetZ());
    }

    void Camera::refactor(float width, float height) {
        projection = glm::perspective(glm::radians(60.f), width/height, .01f, 1000.f);
    }
}
