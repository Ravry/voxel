#include "renderer.h"
#include "window.h"
#include "gizmo.h"
#include "debug_helper.h"

namespace Voxel {
    namespace Game {
        static bool debug {false};

        static std::unique_ptr<Mesh> mesh;
        static std::unique_ptr<Instance3D> instance;

        Renderer::Renderer(float width, float height) {
            Shader::create_shader(
                "default",
                ASSETS_DIR "shaders/default/vert.glsl",
                ASSETS_DIR "shaders/default/frag.glsl"
            );

            Shader::create_shader(
                "greedy",
                ASSETS_DIR "shaders/greedy-mesh/vert.glsl",
                ASSETS_DIR "shaders/greedy-mesh/frag.glsl"
            );

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

            Noise noise;
            const int TEX_2_SIZE {100};
            uint8_t pixels[TEX_2_SIZE * TEX_2_SIZE];
            for (int x {0}; x < TEX_2_SIZE; x++) {
                for (int y {0}; y < TEX_2_SIZE; y++) {
                    pixels[x + y * TEX_2_SIZE] = (uint8_t)(noise.fetch(x * 4, y * 4, 0) * 255);
                }
            }

            Texture::TextureCreateInfo texture_create_info_2 {GL_TEXTURE_2D};
            texture_create_info_2.width = TEX_2_SIZE;
            texture_create_info_2.height = TEX_2_SIZE;
            texture_create_info_2.data_buffer = pixels;
            Texture texture_2(texture_create_info_2);
            texture_2.bind();

            mesh = std::make_unique<Mesh>(PrimitiveType::Triangle);
            instance = std::make_unique<Instance3D>(mesh.get(), glm::vec3(1), glm::vec3(0, 20, 0), glm::vec3(10.f));

            camera = std::make_unique<Camera>(width, height, glm::vec3(0, 16, 0));
            chunk_manager = std::make_unique<ChunkManager>();

            glEnable(GL_DEPTH_TEST);
            glLineWidth(2.f);
            glClearColor(.4f, .4f, 1.f, 1.f);

            Gizmo::setup_axis_gizmo(vao_axis_gizmo);
        }

        void Renderer::update(GLFWwindow* window, float delta_time) {
            camera->update(window, delta_time);

            if (Input::is_key_pressed(GLFW_KEY_X)) {
                debug = !debug;
                glPolygonMode(GL_FRONT_AND_BACK, debug ? GL_LINE : GL_FILL);
            }

            if (Input::is_key_pressed(GLFW_KEY_R)) {
                std::cout << "[INFO] shaders reloaded ... " << std::endl;
            }
        }

        void Renderer::render() {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Shader::get_shader("greedy")
                ->use()
                ->set_uniform_mat4("view", camera->get_matrix())
                ->set_uniform_mat4("projection", camera->get_projection());

            chunk_manager->update(camera->position);
            chunk_manager->render_chunk_compounds();

            Shader::get_shader("default")
                ->use()
                ->set_uniform_mat4("view", camera->get_matrix())
                ->set_uniform_mat4("projection", camera->get_projection());

            instance->render(*Shader::get_shader("default"));

            if (debug) {
                Gizmo::render_axis_gizmo(vao_axis_gizmo, *camera);
            }
        }

        void Renderer::refactor(int width, int height) {
            glViewport(0, 0, width, height);
            camera->refactor(width, height);
        }

        void Renderer::cleanup() {}

    }
}
