#include "core/window.h"

namespace Voxel {
    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam)
    {
        if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        LOG("======================");

        LOG("DEBUG: ({}): {}", id, message);
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             LOG("Source: API"); break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   LOG("Source: Window System"); break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: LOG("Source: Shader Compiler"); break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     LOG("Source: Third Party"); break;
            case GL_DEBUG_SOURCE_APPLICATION:     LOG("Source: Application"); break;
            case GL_DEBUG_SOURCE_OTHER:           LOG("Source: Other"); break;
        }

        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:               LOG("Type: Error"); break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: LOG("Type: Deprecated Behaviour"); break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  LOG("Type: Undefined Behaviour"); break;
            case GL_DEBUG_TYPE_PORTABILITY:         LOG("Type: Portability"); break;
            case GL_DEBUG_TYPE_PERFORMANCE:         LOG("Type: Performance"); break;
            case GL_DEBUG_TYPE_MARKER:              LOG("Type: Marker"); break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          LOG("Type: Push Group"); break;
            case GL_DEBUG_TYPE_POP_GROUP:           LOG("Type: Pop Group"); break;
            case GL_DEBUG_TYPE_OTHER:               LOG("Type: Other"); break;
        }

        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         LOG("Severity: high"); break;
            case GL_DEBUG_SEVERITY_MEDIUM:       LOG("Severity: medium"); break;
            case GL_DEBUG_SEVERITY_LOW:          LOG("Severity: low"); break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: LOG("Severity: notification"); break;
        }

        LOG("======================");
    }

    Window::Window(int width, int height, std::string_view title)
    {
        if (!glfwInit()) return;

        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        glfwSetWindowPos(window, 100, 100);

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        glfwSwapInterval(0);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetKeyCallback(window, Input::key_callback);
        glfwSetMouseButtonCallback(window, Input::mouse_button_callback);
        glfwSetCursorPosCallback(window, Input::mouse_callback);
        glfwSetWindowSizeCallback(window, [] (GLFWwindow* window, int width, int height) {
            while (width <= 0 || height <= 0) {
                glfwWaitEvents();
                glfwGetWindowSize(window, &width, &height);
            }

            // renderer->refactor(width, height);
        });

        LOG("renderer: {}"  ,   (const char*)glGetString(GL_RENDERER));
        LOG("vendor: {}"    ,   (const char*)glGetString(GL_VENDOR));
        LOG("version: {}"   ,   (const char*)glGetString(GL_VERSION));

        renderer = std::make_unique<Game::Renderer>(window, width, height);
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Window::run()
    {
        while (!glfwWindowShouldClose(window)) {
            if (Input::is_key_pressed(GLFW_KEY_C)) glfwSetWindowShouldClose(window, GLFW_TRUE);

            static double last_time = glfwGetTime();
            double time = glfwGetTime();
            Time::delta_time = time - last_time;
            last_time = time;

            Input::update();
            glfwPollEvents();

            renderer->update(window, Time::delta_time);
            renderer->render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }
}