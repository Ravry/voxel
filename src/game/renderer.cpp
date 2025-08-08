#include "renderer.h"
#include "window.h"
#include "gizmo.h"
#include "debug_helper.h"

namespace Voxel {
    namespace Game {
        static bool debug {false};

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
            std::map<unsigned int, std::vector<std::string_view>> block_type_path_map {
                { 0, { ASSETS_DIR "textures/double_checkered.png" } },
                { 1, { ASSETS_DIR "textures/dirt.png" } },
                { 2, { ASSETS_DIR "textures/stone.png"} },
                { 3, { ASSETS_DIR "textures/snow.png"} },
                { 4, { ASSETS_DIR "textures/dirt.png", ASSETS_DIR "textures/grass_side.png", ASSETS_DIR "textures/grass.png" } },
            };

            Texture::TextureCreateInfo texture_create_info {
                .target = GL_TEXTURE_2D_ARRAY,
                .width = TEXTURE_SIZE,
                .height = TEXTURE_SIZE,
                .layer_path_map = block_type_path_map,
                .num_textures = 7,
            };

            Texture texture(texture_create_info);
            texture.bind();

            camera = std::make_unique<Camera>(width, height, glm::vec3(0, 16, 0));
            chunk_manager = std::make_unique<ChunkManager>(camera->position);

            glEnable (GL_CULL_FACE);
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
