#include "renderer.h"
#include "window.h"
#include "gizmo.h"
#include "debug_helper.h"

namespace Voxel::Game {
    static bool debug {false};

    static std::unique_ptr<VAO> vao;
    static std::unique_ptr<VBO> vbo;
    static std::unique_ptr<EBO> ebo;
    static glm::mat4 mat;

    void* allocate_buffer_of_size(size_t size) {
        vbo = std::make_unique<VBO>();
        vbo->bind();

        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, flags);
        return (void*)glMapBufferRange(GL_ARRAY_BUFFER, 0, size, flags);
    }

    void init_some_shit() {}

    void draw_some_shit() {}

    static std::unique_ptr<FBO> framebuffer;
    static std::unique_ptr<VAO> framebuffer_vao;
    static std::unique_ptr<VBO> framebuffer_vbo;
    static std::unique_ptr<EBO> framebuffer_ebo;

    Renderer::Renderer(GLFWwindow* window, float width, float height) {
        #pragma region imgui_init
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
        #pragma endregion

        init_some_shit();

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

        ResourceManager::create_resource<Shader>(
            "framebuffer",
            ASSETS_DIR "shaders/framebuffer/vert.glsl",
            ASSETS_DIR "shaders/framebuffer/frag.glsl"
        );

        Texture::TextureCreateInfo framebuffer_color_attachment_create_info {};
        framebuffer_color_attachment_create_info.target = GL_TEXTURE_2D;
        framebuffer_color_attachment_create_info.internal_format = GL_RGB;
        framebuffer_color_attachment_create_info.format = GL_RGB;
        framebuffer_color_attachment_create_info.type = GL_UNSIGNED_BYTE;
        framebuffer_color_attachment_create_info.width = (unsigned int)width;
        framebuffer_color_attachment_create_info.height = (unsigned int)height;
        framebuffer_color_attachment_create_info.min_filter = GL_LINEAR;
        framebuffer_color_attachment_create_info.mag_filter = GL_LINEAR;
        auto& framebuffer_color_attachment_texture = ResourceManager::create_resource<Texture>("framebuffer_color_attachment_texture", framebuffer_color_attachment_create_info);

        Texture::TextureCreateInfo framebuffer_depth_stencil_attachment_create_info {};
        framebuffer_depth_stencil_attachment_create_info.target = GL_TEXTURE_2D;
        framebuffer_depth_stencil_attachment_create_info.internal_format = GL_DEPTH24_STENCIL8;
        framebuffer_depth_stencil_attachment_create_info.format = GL_DEPTH_STENCIL;
        framebuffer_depth_stencil_attachment_create_info.type = GL_UNSIGNED_INT_24_8;
        framebuffer_depth_stencil_attachment_create_info.width = (unsigned int)width;
        framebuffer_depth_stencil_attachment_create_info.height = (unsigned int)height;
        framebuffer_depth_stencil_attachment_create_info.min_filter = GL_LINEAR;
        framebuffer_depth_stencil_attachment_create_info.mag_filter = GL_LINEAR;

        auto& framebuffer_depth_stencil_attachment_texture = ResourceManager::create_resource<Texture>("framebuffer_depth_stencil_attachment_texture", framebuffer_depth_stencil_attachment_create_info);

        framebuffer = std::make_unique<FBO>();

        framebuffer->bind();
        framebuffer->attach(GL_COLOR_ATTACHMENT0, &framebuffer_color_attachment_texture);
        framebuffer->attach(GL_DEPTH_STENCIL_ATTACHMENT, &framebuffer_depth_stencil_attachment_texture);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            switch (status) {
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    LOG("error -> GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    LOG("error -> GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    LOG("error -> GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    LOG("error -> GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    LOG("error -> GL_FRAMEBUFFER_UNSUPPORTED");
                    break;
                default:
                    LOG("error -> Unknown framebuffer error: {}", std::to_string(status));
            }
        }
        framebuffer->unbind();

        framebuffer_vao = std::make_unique<VAO>();
        framebuffer_vbo = std::make_unique<VBO>();
        framebuffer_ebo = std::make_unique<EBO>();

        framebuffer_vao->bind();
        framebuffer_vbo->bind();
        framebuffer_ebo->bind();

        framebuffer_vbo->data(Geometry::Quad::vertices, sizeof(Geometry::Quad::vertices), GL_STATIC_DRAW);
        framebuffer_ebo->data(Geometry::Quad::indices, sizeof(Geometry::Quad::indices), GL_STATIC_DRAW);
        framebuffer_vao->attrib(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        framebuffer_vao->attrib(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        framebuffer_vao->unbind();
        framebuffer_vbo->unbind();
        framebuffer_ebo->unbind();

        const unsigned int TEXTURE_SIZE = 16;
        std::map<unsigned int, std::vector<std::string_view>> block_type_path_map {
            { BlockType::Dirt       >> 1, { ASSETS_DIR "textures/dirt.png" } },
            { BlockType::Stone      >> 1, { ASSETS_DIR "textures/stone.png"} },
            { BlockType::Snow       >> 1, { ASSETS_DIR "textures/snow.png"} },
            { BlockType::Grass      >> 1, { ASSETS_DIR "textures/dirt.png", ASSETS_DIR "textures/grass_side.png", ASSETS_DIR "textures/grass.png" } },
            { BlockType::Wood       >> 1, { ASSETS_DIR "textures/wood_top.png", ASSETS_DIR "textures/wood.png", ASSETS_DIR "textures/wood_top.png"} },
            { BlockType::Leafs      >> 1, { ASSETS_DIR "textures/leafs.png" } },
            { BlockType::Diamond    >> 1, { ASSETS_DIR "textures/diamond.png" } },
            { BlockType::Bedrock    >> 1, { ASSETS_DIR "textures/double_checkered.png" } },
        };
        Texture::TextureCreateInfo texture_create_info {
            .target = GL_TEXTURE_2D_ARRAY,
            .width = TEXTURE_SIZE,
            .height = TEXTURE_SIZE,
            .layer_path_map = block_type_path_map,
            .num_textures = 13,
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

        Gizmo::setup_axis_gizmo(vao_axis_gizmo);

        glEnable(GL_CULL_FACE);
        glLineWidth(2.f);
        glClearColor(.4f, .4f, 1.f, 1.f);
        glViewport(0, 0, width, height);
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

    void Renderer::draw_imgui_stuff() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("statistics");

        ImGui::Text((std::to_string(1./Time::Timer::delta_time) + std::string(" fps (") + std::to_string(Time::Timer::delta_time * 1000.) + std::string(" ms)")).c_str());
        ImGui::Text(std::format("cam: x={}; y={}; z={}", camera->position.x, camera->position.y, camera->position.z).c_str());
        ImGui::Checkbox("show_gizmos", &Gizmo::show_gizmos);

        ImGui::End();

        ImGui::Render();
    }


    void Renderer::render() {
        framebuffer->bind();
        glEnable(GL_DEPTH_TEST);
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

        if (debug) Gizmo::render_axis_gizmo(vao_axis_gizmo, *camera);

        glDisable(GL_DEPTH_TEST);
        framebuffer->unbind();
        glClear(GL_COLOR_BUFFER_BIT);

        ResourceManager::get_resource<Shader>("framebuffer").use();
        framebuffer_vao->bind();
        framebuffer->texture_attachments[0]->bind();
        glDrawElements(GL_TRIANGLES, sizeof(Geometry::Quad::indices)/sizeof(Geometry::Quad::indices[0]), GL_UNSIGNED_INT, 0);

        draw_imgui_stuff();
    }

    void Renderer::refactor(int width, int height) {
        glViewport(0, 0, width, height);
        camera->refactor(width, height);
    }
}