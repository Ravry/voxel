#include "renderer.h"
#include "window.h"
#include "gizmo.h"
#include "debug_helper.h"

namespace Voxel::Game {
    static bool debug {false};

    static std::unique_ptr<FBO> msaa_framebuffer;
    static std::unique_ptr<FBO> intermediate_framebuffer;

    static FBO::FramebufferAttachment framebuffer_color_attachment;
    static FBO::FramebufferAttachment framebuffer_depth_stencil_attachment;

    static FBO::FramebufferAttachment framebuffer_color_attachment2;

    static std::unique_ptr<VAO> framebuffer_vao;
    static std::unique_ptr<VBO> framebuffer_vbo;
    static std::unique_ptr<EBO> framebuffer_ebo;

    void create_attachments_for_msaa_framebuffer(unsigned int  width, unsigned int height) {
        Texture::TextureCreateInfo framebuffer_color_attachment_create_info {};
        framebuffer_color_attachment_create_info.target = GL_TEXTURE_2D_MULTISAMPLE;
        framebuffer_color_attachment_create_info.internal_format = GL_RGB;
        framebuffer_color_attachment_create_info.width = width;
        framebuffer_color_attachment_create_info.height = height;
        framebuffer_color_attachment_create_info.samples = 4;
        auto& framebuffer_color_attachment_texture = ResourceManager::create_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT, framebuffer_color_attachment_create_info);

        Texture::TextureCreateInfo framebuffer_depth_stencil_attachment_create_info {};
        framebuffer_depth_stencil_attachment_create_info.target = GL_TEXTURE_2D_MULTISAMPLE;
        framebuffer_depth_stencil_attachment_create_info.internal_format = GL_DEPTH24_STENCIL8;
        framebuffer_depth_stencil_attachment_create_info.width = width;
        framebuffer_depth_stencil_attachment_create_info.height = height;
        framebuffer_depth_stencil_attachment_create_info.samples = 4;
        auto& framebuffer_depth_stencil_attachment_texture = ResourceManager::create_resource<Texture>(TEXTURE_FRAMEBUFFER_DEPTH_STENCIL_ATTACHMENT, framebuffer_depth_stencil_attachment_create_info);

        msaa_framebuffer->bind();
        framebuffer_color_attachment = {
            GL_COLOR_ATTACHMENT0, framebuffer_color_attachment_create_info.target, &framebuffer_color_attachment_texture
        };
        msaa_framebuffer->attach(&framebuffer_color_attachment);
        framebuffer_depth_stencil_attachment = {
            GL_DEPTH_STENCIL_ATTACHMENT, framebuffer_depth_stencil_attachment_create_info.target, &framebuffer_depth_stencil_attachment_texture
        };
        msaa_framebuffer->attach(&framebuffer_depth_stencil_attachment);

        if (GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            LOG("error -> framebuffer error: {}", std::to_string(status).c_str());

        msaa_framebuffer->unbind();
    }

    void create_attachments_for_intermediate_framebuffer(unsigned int width, unsigned int height) {
        Texture::TextureCreateInfo framebuffer_color_attachment_create_info {};
        framebuffer_color_attachment_create_info.target = GL_TEXTURE_2D;
        framebuffer_color_attachment_create_info.internal_format = GL_RGB;
        framebuffer_color_attachment_create_info.format = GL_RGB;
        framebuffer_color_attachment_create_info.type = GL_UNSIGNED_BYTE;
        framebuffer_color_attachment_create_info.width = (unsigned int)width;
        framebuffer_color_attachment_create_info.height = (unsigned int)height;
        framebuffer_color_attachment_create_info.min_filter = GL_LINEAR;
        framebuffer_color_attachment_create_info.mag_filter = GL_LINEAR;
        auto& framebuffer_color_attachment_texture = ResourceManager::create_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2, framebuffer_color_attachment_create_info);

        intermediate_framebuffer->bind();
        framebuffer_color_attachment2 = {
            GL_COLOR_ATTACHMENT0, framebuffer_color_attachment_create_info.target, &framebuffer_color_attachment_texture
        };
        intermediate_framebuffer->attach(&framebuffer_color_attachment2);

        if (GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            LOG("error -> framebuffer error: {}", std::to_string(status).c_str());

        intermediate_framebuffer->unbind();
    }

    Renderer::Renderer(GLFWwindow* window, float width, float height) : width(width), height(height) {
        //IMGUI-INIT
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 330 core");
        }

        //SHADER-INIT
        {
            ResourceManager::create_resource<Shader>(
                SHADER_DEFAULT,
                ASSETS_DIR "shaders/default/vert.glsl",
                ASSETS_DIR "shaders/default/frag.glsl"
            );

            ResourceManager::create_resource<Shader>(
                SHADER_GREEDY_MESH,
                ASSETS_DIR "shaders/greedy-mesh/vert.glsl",
                ASSETS_DIR "shaders/greedy-mesh/frag.glsl"
            );

            ResourceManager::create_resource<Shader>(
                SHADER_FRAMEBUFFER,
                ASSETS_DIR "shaders/framebuffer/vert.glsl",
                ASSETS_DIR "shaders/framebuffer/frag.glsl"
            );
        }

        //FRAMEBUFFER-INIT
        {
            msaa_framebuffer = std::make_unique<FBO>();
            create_attachments_for_msaa_framebuffer(width, height);

            intermediate_framebuffer = std::make_unique<FBO>();
            create_attachments_for_intermediate_framebuffer(width, height);

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

        }

        //GREEDY-MESH-TEXTURE-INIT
        {
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
            ResourceManager::create_resource<Texture>("greedy_texture_array", texture_create_info).bind();
        }

        //GENERAL-INIT
        {
            Gizmo::setup_axis_gizmo(vao_axis_gizmo);
            camera = &ResourceManager::create_resource<Camera>("camera_game", width, height, glm::vec3(0, 64, 0));
            chunk_manager = std::make_unique<ChunkManager>(camera->position);
        }

        //GL-INIT
        {
            glEnable(GL_CULL_FACE);
            if (OPTION_MULTISAMPLING_ENABLED) glEnable(GL_MULTISAMPLE);
            glDisable(GL_BLEND);

            glLineWidth(2.f);
            glViewport(0, 0, width, height);
            glClearColor(.4f, .4f, 1.f, 1.f);
        }
    }

    void Renderer::update(GLFWwindow* window, float delta_time) {
        camera->update(window, delta_time);

        if (Input::is_key_pressed(GLFW_KEY_X)) {
            debug = !debug;
        }

        if (Input::is_key_pressed(GLFW_KEY_R)) {
            for (auto& shader : ResourceManager::get_storage<Shader>()) {
                shader.second->reload();
                LOG("{} reloaded ...", shader.first);
            }
        }
    }

    void Renderer::draw_imgui_stuff() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("statistics");

        ImGui::Text(renderer_c_str);
        ImGui::Text((std::to_string(1./Time::Timer::delta_time) + std::string(" fps (") + std::to_string(Time::Timer::delta_time * 1000.) + std::string(" ms)")).c_str());
        ImGui::Text(std::format("cam: x={:.2f}; y={:.2f}; z={:.2f}", camera->position.x, camera->position.y, camera->position.z).c_str());
        ImGui::Checkbox("show_gizmos", &Gizmo::show_gizmos);

        ImGui::End();

        ImGui::Render();
    }

    void Renderer::render() {
        //RENDER-PASS
        {
            msaa_framebuffer->bind();
            glEnable(GL_DEPTH_TEST);
            glDepthRange(0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ResourceManager::get_resource<Shader>(SHADER_GREEDY_MESH)
                .use()
                .set_uniform_mat4("view", camera->get_matrix())
                .set_uniform_mat4("projection", camera->get_projection());

            chunk_manager->update(camera->position);
            chunk_manager->render_chunk_compounds(*camera);

            ResourceManager::get_resource<Shader>(SHADER_DEFAULT)
                .use()
                .set_uniform_mat4("view", camera->get_matrix())
                .set_uniform_mat4("projection", camera->get_projection())
                .set_uniform_int("use_texture", GL_FALSE);

            if (debug) Gizmo::render_axis_gizmo(vao_axis_gizmo, *camera);

            glDisable(GL_DEPTH_TEST);
            msaa_framebuffer->unbind();
        }

        //FRAMEBUFFER-BLITING
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_framebuffer->get_id());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediate_framebuffer->get_id());
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //FRAMEBUFFER-PASS
        {
            glClear(GL_COLOR_BUFFER_BIT);
            ResourceManager::get_resource<Shader>(SHADER_FRAMEBUFFER).use();
            framebuffer_vao->bind();
            std::get<Texture*>(intermediate_framebuffer->attachments[0]->attachment_buffer)->bind();
            glDrawElements(GL_TRIANGLES, sizeof(Geometry::Quad::indices)/sizeof(Geometry::Quad::indices[0]), GL_UNSIGNED_INT, 0);
            draw_imgui_stuff();
        }
    }

    void Renderer::refactor(int width, int height) {
        if (this->width == width && this->height == height) return;

        this->width = width;
        this->height = height;

        glViewport(0, 0, width, height);
        camera->refactor(width, height);
        create_attachments_for_msaa_framebuffer(width, height);
        create_attachments_for_intermediate_framebuffer(width, height);
    }
}