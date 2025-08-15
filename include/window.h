#pragma once
#include <string_view>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Voxel {
    extern const char* renderer_c_str;

    class Window {
    public:
        Window(int width, int height, std::string_view title);
        ~Window();
        void run();

    private:
        GLFWwindow* window;
    };
}