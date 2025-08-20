#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "engine/time.h"

namespace Voxel {
    class Input {
    public:
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

        static bool is_key_pressed(int key);
        static bool is_key_held_down(int key);

        static void update();
    private:
    public:
        static float delta_x;
        static float delta_y;
        static float sensitivity;
    };
}
