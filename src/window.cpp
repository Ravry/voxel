#include <vector>
#include <bitset>
#include "window.h"
#include "camera.h"
#include "instance3D.h"
#include "noise.h"
#include "texture.h"

namespace Voxel {    
    static Camera* camera {nullptr};
    static double delta_time {0};
    static bool debug {false};
    static std::vector<Shader*> shaders;

    Window::Window(int width, int height, std::string_view title)
    {
        if (!glfwInit()) return;

        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS) {
                if (key == GLFW_KEY_ESCAPE)
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                if (key == GLFW_KEY_X)
                {
                    debug = !debug;
                    glPolygonMode(GL_FRONT_AND_BACK, debug ? GL_LINE : GL_FILL);
                }
                if (key == GLFW_KEY_R)
                {
                    for (auto shader : shaders) {
                        shader->reload();
                        std::cout << "shaders reloaded!\n";
                    }
                }
            }
        });
        
        glfwSetCursorPosCallback(window, [] (GLFWwindow* window, double xpos, double ypos) {
            static double last_xpos, last_ypos;
            static bool first_mouse = true;

            if (first_mouse) {
                last_xpos = xpos;
                last_ypos = ypos;
                first_mouse = false;
            }

            static float sensitivity = 10.f;
            float delta_x = (xpos - last_xpos) * sensitivity * delta_time;
            float delta_y = (ypos - last_ypos) * sensitivity * delta_time;

            camera->mouse_moved(delta_x, delta_y);
            
            last_xpos = xpos;
            last_ypos = ypos;
        });

        glfwSetWindowSizeCallback(window, [] (GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
            camera->refactor(width, height);
        });

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        camera = new Camera(width, height);
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void setup_axis_gizmo(VAO& vao) {
        VBO vbo;
        EBO ebo;
        vao.bind();
        vbo.bind();
        ebo.bind();
        float line_vertices[] {
            0, 0, 0,
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        };
        vbo.data(line_vertices, sizeof(line_vertices));
        unsigned int line_indices[] {
            0, 1,
            0, 2,
            0, 3
        };
        ebo.data(line_indices, sizeof(line_indices));
        vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    void render_axis_gizmo(VAO& vao, Shader& shader) {
         glDisable(GL_DEPTH_TEST);

        glm::mat4 model = glm::scale(glm::translate(glm::mat4(1), (camera->position + camera->front)), glm::vec3(.05f));
        shader.set_uniform_mat4("model", model);
        vao.bind();
        shader.set_uniform_vec3("albedo", glm::vec3(1, 0, 0));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)0);
        shader.set_uniform_vec3("albedo", glm::vec3(0, 1, 0));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(unsigned int)));
        shader.set_uniform_vec3("albedo", glm::vec3(0, 0, 1));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(unsigned int)));
        vao.unbind();

        glEnable(GL_DEPTH_TEST);
    }

    void Window::run()
    {
        Shader shader(ASSETS_DIR "shaders/default/vert.glsl", ASSETS_DIR "shaders/default/frag.glsl");
        shaders.push_back(&shader);
        std::vector<Instance3D> instances;
        Mesh cube_mesh(Cube);

        Noise noise;

        uint8_t voxels[SIZE * SIZE * 3] = {};

        for (uint8_t y = 0; y < SIZE; y++)
        {
            for (uint8_t z = 0; z < SIZE; z++)
            {
                for (uint8_t x = 0; x < SIZE; x++)
                {
                    uint8_t& row1 = voxels[z + (y * SIZE)];
                    uint8_t& row2 = voxels[x + (y * SIZE) + (SIZE * SIZE)];
                    uint8_t& row3 = voxels[x + (z * SIZE) + ((SIZE * SIZE) * 2)];

                    uint8_t noise_value =  noise.fetch(x, y, z);
                    if (noise_value)
                        instances.push_back(Instance3D(&cube_mesh, glm::vec3((float)noise_value), glm::vec3(x, y, z), glm::vec3(.2f)));

                    row1 |= noise_value << x;
                    row2 |= noise_value << z;
                    row3 |= noise_value << y;
                }
            }
        }
            
        
        Mesh greedy_mesh(voxels, SIZE);
        instances.push_back(Instance3D(&greedy_mesh, glm::vec3(0, 0, 0), glm::vec3(0)));

        VAO vao;
        setup_axis_gizmo(vao);

        Texture texture(ASSETS_DIR "textures/checkered.png");
        texture.bind();

        glLineWidth(2.f);
        glClearColor(.4f, .4f, 1.f, 1.f);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            static double last_time = glfwGetTime();
            double time = glfwGetTime();
            delta_time = time - last_time;
            last_time = time;

            camera->update(window, (float)delta_time);
    
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            shader.set_uniform_mat4("view", camera->get_matrix()).set_uniform_mat4("projection", camera->get_projection());
            
            for (auto& instance : instances) {
                instance.render(shader);
            }

            render_axis_gizmo(vao, shader);
            
            shader.unuse();

            glfwSwapBuffers(window);
        }
        shader.destroy();
    }
}