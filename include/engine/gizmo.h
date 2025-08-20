#pragma once
#include "engine/buffer.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/mesh.h"
#include "engine/resource_manager.h"
#include "game/misc.h"

namespace Voxel::Gizmo {
    extern bool show_gizmos;

    void setup_line_box_gizmo(VAO* vao);
    void render_line_box_gizmo(glm::vec3 position, glm::vec3 size);
    void setup_axis_gizmo(VAO* vao);
    void render_axis_gizmo(Camera& camera);
}