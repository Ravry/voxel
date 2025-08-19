#pragma once
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "resource_manager.h"
#include "noise.h"
#include "misc.h"
#include "texture.h"
#include "camera.h"
#include "chunk_manager.h"
#include "physics_manager.h"

namespace Voxel {
    namespace Game {
        class Renderer {
        public:
            Renderer(GLFWwindow* window, float width, float height);
            void update(GLFWwindow* window, float delta_time);
            void draw_imgui_stuff();
            void render();
            void refactor(int width, int height);
            void render_axis_gizmo(VAO& vao, Shader& shader);
            void setup_axis_gizmo(VAO& vao);
        private:
            unsigned int width, height;
            std::unique_ptr<ChunkManager> chunk_manager;
            Camera* camera;
            VAO vao_axis_gizmo;
            std::unique_ptr<UBO> matrices_ubo;
            Physics::PhysicsManager* physics_manager;
        };
    }
}
