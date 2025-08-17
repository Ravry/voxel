#include "renderer.h"
#include "window.h"
#include "gizmo.h"
#include "debug_helper.h"

namespace Voxel::Game {
    static bool debug {false};

    constexpr unsigned int SHADOW_MAP_SIZE = 4096;
    static glm::mat4 shadow_map_projection_matrix;
    static glm::mat4 shadow_map_view_matrix;

    static std::unique_ptr<FBO> msaa_framebuffer;
    static std::unique_ptr<FBO> intermediate_framebuffer;
    static std::unique_ptr<FBO> shadow_map_fbo;

    static FBO::FramebufferAttachment framebuffer_color_attachment;
    static FBO::FramebufferAttachment framebuffer_depth_stencil_attachment;
    static FBO::FramebufferAttachment framebuffer_color_attachment2;

    static FBO::FramebufferAttachment framebuffer_color_map_attachment;
    static FBO::FramebufferAttachment framebuffer_shadow_map_attachment;

    static std::unique_ptr<VAO> framebuffer_vao;
    static std::unique_ptr<VBO> framebuffer_vbo;
    static std::unique_ptr<EBO> framebuffer_ebo;

    static std::unique_ptr<VAO> skybox_vao;
    static std::unique_ptr<VBO> skybox_vbo;
    static std::unique_ptr<EBO> skybox_ebo;

    static std::unique_ptr<UBO> matrices_ubo;

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

    void create_attachments_for_shadow_map_framebuffer(unsigned int width, unsigned int height) {
        Texture::TextureCreateInfo framebuffer_shadow_map_attachment_create_info {};
        framebuffer_shadow_map_attachment_create_info.target = GL_TEXTURE_2D;
        framebuffer_shadow_map_attachment_create_info.internal_format = GL_DEPTH_COMPONENT;
        framebuffer_shadow_map_attachment_create_info.format = GL_DEPTH_COMPONENT;
        framebuffer_shadow_map_attachment_create_info.type = GL_FLOAT;
        framebuffer_shadow_map_attachment_create_info.width = (unsigned int)width;
        framebuffer_shadow_map_attachment_create_info.height = (unsigned int)height;
        framebuffer_shadow_map_attachment_create_info.min_filter = GL_NEAREST;
        framebuffer_shadow_map_attachment_create_info.mag_filter = GL_NEAREST;
        framebuffer_shadow_map_attachment_create_info.wrap = GL_CLAMP_TO_BORDER;

        auto& framebuffer_shadow_map_attachment_texture = ResourceManager::create_resource<Texture>(TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT, framebuffer_shadow_map_attachment_create_info);

        Texture::TextureCreateInfo framebuffer_color_map_attachment_create_info {};
        framebuffer_color_map_attachment_create_info.target = GL_TEXTURE_2D;
        framebuffer_color_map_attachment_create_info.internal_format = GL_RGB;
        framebuffer_color_map_attachment_create_info.format = GL_RGB;
        framebuffer_color_map_attachment_create_info.type = GL_UNSIGNED_BYTE;
        framebuffer_color_map_attachment_create_info.width = (unsigned int)width;
        framebuffer_color_map_attachment_create_info.height = (unsigned int)height;
        framebuffer_color_map_attachment_create_info.min_filter = GL_NEAREST;
        framebuffer_color_map_attachment_create_info.mag_filter = GL_NEAREST;
        framebuffer_color_map_attachment_create_info.wrap = GL_CLAMP_TO_BORDER;

        auto& framebuffer_color_map_attachment_texture = ResourceManager::create_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_MAP_ATTACHMENT, framebuffer_color_map_attachment_create_info);


        shadow_map_fbo->bind();

        framebuffer_shadow_map_attachment = {
            GL_DEPTH_ATTACHMENT, framebuffer_shadow_map_attachment_create_info.target, &framebuffer_shadow_map_attachment_texture
        };

        framebuffer_color_map_attachment = {
            GL_COLOR_ATTACHMENT0, framebuffer_color_map_attachment_create_info.target, & framebuffer_color_map_attachment_texture
        };

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        shadow_map_fbo->attach(&framebuffer_shadow_map_attachment);
        shadow_map_fbo->attach(&framebuffer_color_map_attachment);

        if (GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            LOG("error -> framebuffer error: {}", std::to_string(status).c_str());

        shadow_map_fbo->unbind();
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
                std::unordered_map<unsigned int, std::string_view>{
                    { GL_VERTEX_SHADER, ASSETS_DIR "shaders/default/vert.glsl" },
                    { GL_FRAGMENT_SHADER, ASSETS_DIR "shaders/default/frag.glsl" }
                }
            );

            ResourceManager::create_resource<Shader>(
                SHADER_FRAMEBUFFER,
                std::unordered_map<unsigned int, std::string_view> {
                    { GL_VERTEX_SHADER, ASSETS_DIR "shaders/framebuffer/vert.glsl" },
                    { GL_FRAGMENT_SHADER, ASSETS_DIR "shaders/framebuffer/frag.glsl" }
                }
            );

            ResourceManager::create_resource<Shader>(
                SHADER_SKYBOX_CUBEMAP,
                std::unordered_map<unsigned int, std::string_view> {
                    { GL_VERTEX_SHADER, ASSETS_DIR "shaders/cubemap/vert.glsl" },
                    { GL_FRAGMENT_SHADER, ASSETS_DIR "shaders/cubemap/frag.glsl" }
                }
            );

            ResourceManager::create_resource<Shader>(
                SHADER_GREEDY_MESH_FOR_SHADOW_PASS,
                std::unordered_map<unsigned int, std::string_view> {
                    { GL_VERTEX_SHADER, ASSETS_DIR "shaders/greedy-mesh/vert.glsl" }
                }
            );

            ResourceManager::create_resource<Shader>(
                SHADER_GREEDY_MESH,
                std::unordered_map<unsigned int, std::string_view> {
                    { GL_VERTEX_SHADER, ASSETS_DIR "shaders/greedy-mesh/vert.glsl" },
                    { GL_FRAGMENT_SHADER, ASSETS_DIR "shaders/greedy-mesh/frag.glsl" },
                }
            );
        }

        //SCREEN-FRAMEBUFFER-INIT
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

        //GENERAL-INIT
        {
            Gizmo::setup_axis_gizmo(vao_axis_gizmo);
            camera = &ResourceManager::create_resource<Camera>("camera_game", width, height, glm::vec3(0, 64, 0));

            chunk_manager = std::make_unique<ChunkManager>(camera->position);
            matrices_ubo = std::make_unique<UBO>(0, nullptr, 2 * sizeof(glm::mat4));
            matrices_ubo->bind();
            matrices_ubo->sub_data((void*)glm::value_ptr(camera->get_projection()), sizeof(glm::mat4), 0);
            matrices_ubo->unbind();
        }

        //SHADOW-FRAMEBUFFER-INIT
        {
            shadow_map_fbo = std::make_unique<FBO>();
            create_attachments_for_shadow_map_framebuffer(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        }

        //SKYBOX-INIT
        {
            Texture::TextureCreateInfo skybox_cubemap_texture_create_info { GL_TEXTURE_CUBE_MAP };
            skybox_cubemap_texture_create_info.wrap = GL_CLAMP_TO_EDGE;
            skybox_cubemap_texture_create_info.min_filter = GL_LINEAR;
            skybox_cubemap_texture_create_info.mag_filter = GL_LINEAR;
            skybox_cubemap_texture_create_info.type = GL_UNSIGNED_BYTE;
            skybox_cubemap_texture_create_info.internal_format = GL_RGB;
            skybox_cubemap_texture_create_info.format = GL_RGBA;
            skybox_cubemap_texture_create_info.cubemap_files = {
                ASSETS_DIR "textures/skybox/px.png",
                ASSETS_DIR "textures/skybox/nx.png",
                ASSETS_DIR "textures/skybox/py.png",
                ASSETS_DIR "textures/skybox/ny.png",
                ASSETS_DIR "textures/skybox/pz.png",
                ASSETS_DIR "textures/skybox/nz.png",
            };
            ResourceManager::create_resource<Texture>(TEXTURE_SKYBOX_CUBEMAP, skybox_cubemap_texture_create_info);

            skybox_vao = std::make_unique<VAO>();
            skybox_vbo = std::make_unique<VBO>();
            skybox_ebo = std::make_unique<EBO>();

            skybox_vao->bind();
            skybox_vbo->bind();
            skybox_ebo->bind();

            skybox_vbo->data(Geometry::Cube::vertices, sizeof(Geometry::Cube::vertices), GL_STATIC_DRAW);
            skybox_ebo->data(Geometry::Cube::indices_inside, sizeof(Geometry::Cube::indices_inside), GL_STATIC_DRAW);
            skybox_vao->attrib(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            skybox_vao->unbind();
            skybox_vbo->unbind();
            skybox_ebo->unbind();
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
        chunk_manager->update(camera->position);

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

        static std::string current_item = TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT;
        static std::unordered_map<std::string, void*> framebuffer_textures {
            { TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2, (void*)(intptr_t)(ResourceManager::get_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2).get_id()) },
            { TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT, (void*)(intptr_t)(ResourceManager::get_resource<Texture>(TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT).get_id()) },
            { TEXTURE_FRAMEBUFFER_COLOR_MAP_ATTACHMENT, (void*)(intptr_t)(ResourceManager::get_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_MAP_ATTACHMENT).get_id()) },
        };

        if (ImGui::BeginCombo("##combo", current_item.c_str())) {
            for (auto& [str, id] : framebuffer_textures) {
                bool is_selected = (current_item == str);
                if (ImGui::Selectable(str.c_str(), is_selected)) {
                    current_item = str;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Image(framebuffer_textures[current_item], ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();

        ImGui::Render();
    }


    struct DirectionalLight {
        glm::vec3 direction;

        DirectionalLight(float rotation_x, float rotation_y, float rotation_z) {
            glm::quat q = glm::quat(glm::vec3(glm::radians(rotation_x), glm::radians(rotation_y), glm::radians(rotation_z)));
            direction = q * glm::vec3(0.0f, 0.0f, -1.0f);
            direction = glm::normalize(direction);
        }
    };


    void Renderer::render() {
        static DirectionalLight directional_light(-50.f, 0, 0);

        glEnable(GL_DEPTH_TEST);

        //SHADOW-RENDER-PASS
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        {
            shadow_map_fbo->bind();

            // LOG("directional_light.direction: {};{};{}", directional_light.direction.x, directional_light.direction.y, directional_light.direction.z);
            const float ortho_size = 50.f;
            shadow_map_projection_matrix = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, 1.0f, 500.f);
            shadow_map_view_matrix = glm::lookAt(
                camera->position - directional_light.direction * 120.f,
                camera->position,
                glm::normalize(glm::cross(directional_light.direction, glm::vec3(1.f, 0.f, 0.f)))
            );

            matrices_ubo->bind();
            matrices_ubo->sub_data((void*)glm::value_ptr(shadow_map_projection_matrix), sizeof(glm::mat4), 0);
            matrices_ubo->sub_data((void*)glm::value_ptr(shadow_map_view_matrix), sizeof(glm::mat4), sizeof(glm::mat4));
            matrices_ubo->unbind();

            glClear(GL_DEPTH_BUFFER_BIT);
            {
                chunk_manager->render_chunk_compounds(*camera, false, ResourceManager::get_resource<Shader>(SHADER_GREEDY_MESH_FOR_SHADOW_PASS));
            }
            shadow_map_fbo->unbind();
        }

        //SCENE-RENDER-PASS
        glViewport(0, 0, width, height);
        {
            msaa_framebuffer->bind();

            matrices_ubo->bind();
            matrices_ubo->sub_data((void*)glm::value_ptr(camera->get_projection()), sizeof(glm::mat4), 0);
            matrices_ubo->sub_data((void*)glm::value_ptr(camera->get_matrix()), sizeof(glm::mat4), sizeof(glm::mat4));
            matrices_ubo->unbind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            {
                glActiveTexture(GL_TEXTURE1);
                std::get<Texture*>(shadow_map_fbo->attachments[0]->attachment_buffer)->bind();
                glm::mat4 light_space_matrix = shadow_map_projection_matrix * shadow_map_view_matrix;
                auto& shader = ResourceManager::get_resource<Shader>(SHADER_GREEDY_MESH)
                    .use()
                    .set_uniform_int("shadow_map", 1)
                    .set_uniform_mat4("light_space_matrix", light_space_matrix)
                    .set_uniform_vec3("light_direction", directional_light.direction);
                glActiveTexture(GL_TEXTURE0);
                chunk_manager->render_chunk_compounds(*camera, true, shader);
            }

            {
                glDepthFunc(GL_LEQUAL);
                if (debug) Gizmo::render_axis_gizmo(vao_axis_gizmo, *camera);
                ResourceManager::get_resource<Shader>(SHADER_SKYBOX_CUBEMAP)
                    .use()
                    .set_uniform_mat4("view_non_translated", glm::mat4(glm::mat3(camera->get_matrix())));

                ResourceManager::get_resource<Texture>(TEXTURE_SKYBOX_CUBEMAP).bind();
                skybox_vao->bind();
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                skybox_vao->unbind();
                glDepthFunc(GL_LESS);
            }

            msaa_framebuffer->unbind();
        }

        //FRAMEBUFFER-BLITING
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_framebuffer->get_id());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediate_framebuffer->get_id());
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //FRAMEBUFFER-PASS
        {
            glDisable(GL_DEPTH_TEST);
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