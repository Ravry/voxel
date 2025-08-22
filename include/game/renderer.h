#pragma once
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "engine/resource_manager.h"
#include "engine/texture.h"
#include "engine/camera.h"
#include "engine/physics_manager.h"
#include "engine/buffer.h"
#include "engine/buffer.h"
#include "engine/gizmo.h"
#include "engine/model.h"
#include "engine/instance_3d.h"
#include "engine/light.h"

#include "game/chunk_manager.h"
#include "game/misc.h"
#include "game/noise.h"

namespace Voxel::Game {
    class Renderer {
    public:
        Renderer(GLFWwindow* window, float width, float height);
        void update(float delta_time);
        void draw_imgui_stuff();
        void render();
        void refactor(int width, int height);
        void render_axis_gizmo(VAO& vao, Shader& shader);
        void setup_axis_gizmo(VAO& vao);
    private:
        unsigned int width, height;
        std::unique_ptr<ChunkManager> chunk_manager;
        std::unique_ptr<UBO> matrices_ubo;
        Camera* camera;
        Physics::PhysicsManager* physics_manager;
    };

}
