#pragma once
#include "buffer.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "resource_manager.h"

namespace Voxel::Gizmo {
    void setup_line_box_gizmo(VAO& vao);
    void render_line_box_gizmo(VAO& vao, glm::vec3 position, glm::vec3 size);
    void setup_axis_gizmo(VAO& vao);
    void render_axis_gizmo(VAO& vao, Camera& camera);
}