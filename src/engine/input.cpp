#include "input.h"
#include <vector>
#include "log.h"

namespace Voxel {
    float Input::delta_x = 0;
    float Input::delta_y = 0;
    float Input::sensitivity = .04f;

    std::vector<int> buttons_pressed;
    bool buttons_down[GLFW_KEY_LAST + 1] = {};

    void Input::key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
        if (action == GLFW_PRESS) {
            buttons_down[key] = true;
            buttons_pressed.push_back(key);
        }
        if (action == GLFW_RELEASE) {
            buttons_down[key] = false;
        }
    }

    void Input::mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
        if (action == GLFW_PRESS) {
            buttons_down[button] = true;
            buttons_pressed.push_back(button);
        }
        if (action == GLFW_RELEASE) {
            buttons_down[button] = false;
        }
    }

    void Input::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
        static double last_xpos, last_ypos;
        static bool first_mouse = true;

        if (first_mouse) {
            last_xpos = xpos;
            last_ypos = ypos;
            first_mouse = false;
        }

        delta_x += (xpos - last_xpos) * sensitivity;
        delta_y += (ypos - last_ypos) * sensitivity;

        last_xpos = xpos;
        last_ypos = ypos;
    }

    void Input::update() {
        delta_x = 0;
        delta_y = 0;
        buttons_pressed.clear();
    }

    bool Input::is_key_pressed(int key) {
        for (auto key_pressed : buttons_pressed)
            if (key_pressed == key)
                return true;

        return false;
    }

    bool Input::is_key_held_down(int key) {
        return buttons_down[key];
    }
}
