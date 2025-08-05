#pragma once
#include "shader.h"
#include "noise.h"
#include "misc.h"
#include "texture.h"
#include "camera.h"
#include "chunk.h"

namespace Voxel {
    namespace Game {
        class Renderer {
        public:
            Renderer(float width, float height);
            void update(GLFWwindow* window, float delta_time);
            void render();
            void cleanup();
            void refactor(int width, int height);
            void render_axis_gizmo(VAO& vao, Shader& shader);
            void setup_axis_gizmo(VAO& vao);
        private:
            std::unique_ptr<Camera> camera;
            std::map<std::string, Shader> shaders;
            std::map<std::string, Instance3D> instances;
            std::vector<std::shared_ptr<Chunk>> chunks;
            VAO vao_axis_gizmo;
            VAO vao_box_gizmo;
            SSBO ssbo;
        };
    }
}