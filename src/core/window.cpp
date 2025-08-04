#include <vector>
#include <bitset>
#include "window.h"
#include "input.h"
#include "renderer.h"

namespace Voxel {
    double Time::Timer::delta_time {0};

    static std::unique_ptr<Game::Renderer> renderer;

    Window::Window(int width, int height, std::string_view title)
    {
        if (!glfwInit()) return;

        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetKeyCallback(window, Input::key_callback);
        glfwSetCursorPosCallback(window, Input::mouse_callback);
        glfwSetWindowSizeCallback(window, [] (GLFWwindow* window, int width, int height) { renderer->refactor(width, height); });

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        renderer = std::make_unique<Game::Renderer>(width, height );
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::run()
    {
        while (!glfwWindowShouldClose(window)) {
            if (Input::is_key_pressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

            static double last_time = glfwGetTime();
            double time = glfwGetTime();
            Time::Timer::delta_time = time - last_time;
            last_time = time;

            Input::update();
            glfwPollEvents();

            renderer->update(window, Time::Timer::delta_time);
            renderer->render();

            glfwSwapBuffers(window);
        }
    }
}