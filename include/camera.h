#pragma once
#include <algorithm>
#include <GLFW/glfw3.h>
#include "transform.h"
#include "input.h"

namespace Voxel {
    struct Plane {
        float a, b, c, d; // Plane equation: ax + by + cz + d = 0
    };

    static void get_frustum(Plane* frustum, glm::mat4 projection, glm::mat4 view) {
        glm::mat4 clip = projection * view;

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

    static bool is_box_in_frustum(const Plane planes[6], const glm::vec3& min, const glm::vec3& max) {
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
            Plane frustum[6];
            glm::vec3 position;
            glm::vec3 front;
        public:
            Camera(float width, float height, glm::vec3 position);
            void update(GLFWwindow* window, float delta_time);
            void refactor(float width, float height);
            glm::mat4 get_projection() { return projection; }
    };
}