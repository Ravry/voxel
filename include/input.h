#pragma once
#include <GLFW/glfw3.h>
#include "utils.h"

namespace Voxel {
    class Input {
    public:
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
        static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

        static bool is_key_pressed(int key);
        static bool is_key_held_down(int key);

        static void update();
    private:
    public:
        static float delta_x;
        static float delta_y;
    };
}
