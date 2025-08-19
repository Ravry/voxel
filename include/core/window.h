#pragma once
#include <string_view>
#include <memory>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "game/renderer.h"

namespace Voxel {
    class Window {
    public:
        Window(int width, int height, std::string_view title);
        ~Window();
        void run();

    private:
        GLFWwindow* window;
        std::unique_ptr<Voxel::Game::Renderer> renderer;
    };
}