#pragma once
#include <string_view>
#include <memory>
#include <vector>
#include <bitset>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "game/renderer.h"
#include "engine/input.h"

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