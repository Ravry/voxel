#pragma once
#include <string_view>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Voxel {
    class Window {
    public:
        Window(int width, int height, std::string_view title);
        ~Window();
        void run();

    private:
        GLFWwindow* window;
    };
}