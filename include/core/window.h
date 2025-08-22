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
        static std::unique_ptr<Window> instance;

        static Window& create_window(int width, int height, std::string_view title) {
            instance = std::make_unique<Window>(width, height, title);
            return *instance;
        }

        static Window& get_instance() {
            return *instance;
        }

        Window(int width, int height, std::string_view title);
        ~Window();
        void run();
        void set_cursor_mode(GLenum mode);

    private:
        GLFWwindow* window;
        std::unique_ptr<Voxel::Game::Renderer> renderer;
    };
}