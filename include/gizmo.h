#pragma once
#include "buffer.h"
#include "shader.h"
#include "camera.h"

namespace Voxel::Gizmo {
    void setup_line_box_gizmo(VAO& vao);
    void render_line_box_gizmo(VAO& vao, Shader& shader, glm::vec3 position, glm::vec3 size);
    void setup_axis_gizmo(VAO& vao);
    void render_axis_gizmo(VAO& vao, Shader& shader, Camera& camera);
}