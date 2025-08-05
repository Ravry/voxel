#include "renderer.h"
#include "window.h"
#include "gizmo.h"

namespace Voxel {
    namespace Game {
        static bool debug {false};

        Renderer::Renderer(float width, float height) {
            camera = std::make_unique<Camera>(width, height);

            static Mesh cube_mesh(Cube);

            shaders["default"] = Shader(
                ASSETS_DIR "shaders/default/vert.glsl",
                ASSETS_DIR "shaders/default/frag.glsl"
            );

            shaders["greedy-mesh"] = Shader(
                ASSETS_DIR "shaders/greedy-mesh/vert.glsl",
                ASSETS_DIR "shaders/greedy-mesh/frag.glsl"
            );

            static Noise noise;

            const size_t chunks_count = 6;
            for (size_t i {0}; i < chunks_count * chunks_count; i++) {
                int x = (i % chunks_count) * SIZE;
                int z = ((i / chunks_count) % chunks_count) * 16;
                Chunk chunk(noise, glm::vec3(x, 0, z));
                chunks.push_back(chunk);
            }

            const unsigned int TEXTURE_SIZE = 16;
            std::map<unsigned int, std::string_view> block_type_path_map {
                { Game::BlockType::Bedrock, ASSETS_DIR "textures/double_checkered.png" },
                { Game::BlockType::Dirt, ASSETS_DIR "textures/dirt.png" },
                { Game::BlockType::Grass, ASSETS_DIR "textures/grass.png" },
            };
            Texture::TextureCreateInfo texture_create_info {
                .target = GL_TEXTURE_2D_ARRAY,
                .width = TEXTURE_SIZE,
                .height = TEXTURE_SIZE,
                .layer_path_map = block_type_path_map
            };
            Texture texture(texture_create_info);
            texture.bind();

            glEnable(GL_DEPTH_TEST);
            glLineWidth(2.f);
            glClearColor(.4f, .4f, 1.f, 1.f);

            Gizmo::setup_axis_gizmo(vao_axis_gizmo);
            Gizmo::setup_line_box_gizmo(vao_box_gizmo);
        }

        void Renderer::update(GLFWwindow* window, float delta_time) {
            camera->update(window, delta_time);

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
                shaders["default"].use();
                if (debug) instance.second.render(shaders["default"]);
            }

            for (auto& chunk : chunks) {
                shaders["greedy-mesh"]
                    .use()
                    .set_uniform_mat4("view", camera->get_matrix())
                    .set_uniform_mat4("projection", camera->get_projection());
                chunk.render(shaders["greedy-mesh"], ssbo);

                shaders["default"].use();
                Gizmo::render_line_box_gizmo(vao_box_gizmo, shaders["default"], chunk.position + glm::vec3(-.01f), glm::vec3(16 + .02f));
            }

            if (debug) {
                shaders["default"].use();
                Gizmo::render_axis_gizmo(vao_axis_gizmo, shaders["default"], *camera);
                shaders["default"].unuse();
            }
        }

        void Renderer::refactor(int width, int height) {
            glViewport(0, 0, width, height);
            camera->refactor(width, height);
        }

        void Renderer::cleanup() {}

    }
}
