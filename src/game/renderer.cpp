#include "renderer.h"

#include "window.h"

namespace Voxel {
    namespace Game {

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

        void Renderer::render_axis_gizmo(VAO& vao, Shader& shader) {
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

        Renderer::Renderer(float width, float height) {
            camera = std::make_unique<Camera>(width, height);

            static Mesh cube_mesh(Cube);
            static Noise noise;

            shaders["default"] = Shader(
                ASSETS_DIR "shaders/default/vert.glsl",
                ASSETS_DIR "shaders/default/frag.glsl"
            );

            shaders["greedy-mesh"] = Shader(
                ASSETS_DIR "shaders/greedy-mesh/vert.glsl",
                ASSETS_DIR "shaders/greedy-mesh/frag.glsl"
            );

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
                            instances["cube" + std::to_string(x + (y * 8) + (z * 8 * 8))] = Instance3D(&cube_mesh, glm::vec3((float)noise_value), glm::vec3(x + .5, y + .5, z + .5), glm::vec3(.2f));

                        row1 |= noise_value << x;
                        row2 |= noise_value << z;
                        row3 |= noise_value << y;
                    }
                }
            }


            static Mesh greedy_mesh(voxels, SIZE);
            instances["greedy"] = Instance3D(&greedy_mesh, glm::vec3(0, 0, 0), glm::vec3(0));

            const unsigned int TEXTURE_SIZE = 16;
            std::map<unsigned int, std::string_view> block_type_path_map {
                { Game::BlockType::Dirt, ASSETS_DIR "textures/dirt.png" },
                { Game::BlockType::Grass, ASSETS_DIR "textures/grass.png" }
            };
            Texture::TextureCreateInfo texture_create_info {
                .target = GL_TEXTURE_2D_ARRAY,
                .width = TEXTURE_SIZE,
                .height = TEXTURE_SIZE,
                .layer_path_map = block_type_path_map
            };

            Texture texture(texture_create_info);
            texture.bind();

            unsigned int data[8 * 8 * 8] {};
            for (int y {0}; y < SIZE; y++) {
                for (int z {0}; z < SIZE; z++) {
                    for (int x {0}; x < SIZE; x++) {
                        data[x + (y * SIZE) + (z * SIZE * SIZE)] = y > 1 && y < 5 ? Game::BlockType::Grass : Game::BlockType::Dirt;
                    }
                }
            }

            SSBO ssbo;
            ssbo.bind();
            ssbo.data(0, &data[0], sizeof(data));
            ssbo.unbind();

            glEnable(GL_DEPTH_TEST);
            glLineWidth(2.f);
            glClearColor(.4f, .4f, 1.f, 1.f);
        }

        void Renderer::update(GLFWwindow* window, float delta_time) {
            camera->update(window, delta_time);

            static bool debug {false};
            if (Input::is_key_pressed(GLFW_KEY_X)) {
                debug = !debug;
                glPolygonMode(GL_FRONT_AND_BACK, debug ? GL_LINE : GL_FILL);
            }

            if (Input::is_key_pressed(GLFW_KEY_R)) {
                for (auto& shader : shaders) {
                    shader.second.reload();
                }
                std::cout << "[INFO] shaders reloaded ... " << std::endl;
            }
        }

        void Renderer::render() {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shaders["default"]
                .use()
                .set_uniform_mat4("view", camera->get_matrix())
                .set_uniform_mat4("projection", camera->get_projection())
                .unuse();

            for (auto& instance : instances) {
                if (instance.first == "greedy") {
                    shaders["greedy-mesh"]
                        .use()
                        .set_uniform_mat4("view", camera->get_matrix())
                        .set_uniform_mat4("projection", camera->get_projection());
                    instance.second.render(shaders["greedy-mesh"]);
                }
                else {
                    shaders["default"].use();
                    instance.second.render(shaders["default"]);
                }
            }
        }

        void Renderer::refactor(int width, int height) {
            glViewport(0, 0, width, height);
            camera->refactor(width, height);
        }

        void Renderer::cleanup() {

        }

    }
}
