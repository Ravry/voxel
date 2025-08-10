#include "renderer.h"
#include "window.h"
#include "gizmo.h"
#include "debug_helper.h"

namespace Voxel {
    namespace Game {
        static bool debug {false};

        Renderer::Renderer(GLFWwindow* window, float width, float height) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 330 core");

            ResourceManager::create_resource<Shader>(
                "default",
                ASSETS_DIR "shaders/default/vert.glsl",
                ASSETS_DIR "shaders/default/frag.glsl"
            );

            ResourceManager::create_resource<Shader>(
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
                { 7, { ASSETS_DIR "textures/wood_top.png", ASSETS_DIR "textures/wood.png", ASSETS_DIR "textures/wood_top.png"} },
                { 10, { ASSETS_DIR "textures/leafs.png" }}
            };
            Texture::TextureCreateInfo texture_create_info {
                .target = GL_TEXTURE_2D_ARRAY,
                .width = TEXTURE_SIZE,
                .height = TEXTURE_SIZE,
                .layer_path_map = block_type_path_map,
                .num_textures = 11,
            };
            ResourceManager::create_resource<Texture>("greedy_texture_array", texture_create_info)
                .bind();

            camera = &ResourceManager::create_resource<Camera>("camera_game", width, height, glm::vec3(0, 64, 0));

            chunk_manager = std::make_unique<ChunkManager>(camera->position);

            auto& quad_mesh = ResourceManager::create_resource<Mesh>("quad_mesh", PrimitiveType::Triangle);
            ResourceManager::create_resource<Instance3D>("quad_instance", &quad_mesh, glm::vec3(1), glm::vec3(0, 32, 0), glm::vec3(10));

            Noise noise;
            const unsigned int NOISE_TEXTURE_SIZE {100};
            uint8_t noise_data[NOISE_TEXTURE_SIZE * NOISE_TEXTURE_SIZE];
            for (int i {0}; i < NOISE_TEXTURE_SIZE; i++) {
                for (int j {0}; j < NOISE_TEXTURE_SIZE; j++) {
                    noise_data[i + (j * NOISE_TEXTURE_SIZE)] = noise.fetch_heightmap(i * 5.f, j* 5.f) * 255;
                }
            }
            Texture::TextureCreateInfo noise_texture_create_info {
                .target = GL_TEXTURE_2D,
                .width = NOISE_TEXTURE_SIZE,
                .height = NOISE_TEXTURE_SIZE,
                .min_filter = GL_LINEAR_MIPMAP_LINEAR,
                .mag_filter = GL_LINEAR,
                .data_buffer = noise_data
            };
            ResourceManager::create_resource<Texture>("noise_texture", noise_texture_create_info).bind();


            glEnable (GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glLineWidth(2.f);
            glClearColor(.4f, .4f, 1.f, 1.f);
            glEnable(GL_DEPTH_TEST);
            glDepthRange(0.0, 1.0);

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

        void render_imgui_stuff() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("statistics");

            ImGui::Text((std::to_string(1./Time::Timer::delta_time) + std::string(" fps (") + std::to_string(Time::Timer::delta_time * 1000.) + std::string(" ms)")).c_str());

            ImGui::End();

            ImGui::Render();
        }

        void Renderer::render() {
            render_imgui_stuff();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ResourceManager::get_resource<Shader>("greedy")
                .use()
                .set_uniform_mat4("view", camera->get_matrix())
                .set_uniform_mat4("projection", camera->get_projection());

            chunk_manager->update(camera->position);
            chunk_manager->render_chunk_compounds(*camera);

            auto& shader = ResourceManager::get_resource<Shader>("default")
                .use()
                .set_uniform_mat4("view", camera->get_matrix())
                .set_uniform_mat4("projection", camera->get_projection())
                .set_uniform_int("use_texture", GL_TRUE);

            ResourceManager::get_resource<Instance3D>("quad_instance").render(shader);
            shader.set_uniform_int("use_texture", GL_FALSE);

            if (debug) {
                Gizmo::render_axis_gizmo(vao_axis_gizmo, *camera);
            }
        }

        void Renderer::refactor(int width, int height) {
            glViewport(0, 0, width, height);
            camera->refactor(width, height);
        }
    }
}
