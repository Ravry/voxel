#include "game/renderer.h"

#include "glm/gtc/type_ptr.hpp"


namespace Voxel::Game {
    static bool debug {false};

    static std::unique_ptr<FBO> msaa_framebuffer;
    static std::unique_ptr<FBO> intermediate_framebuffer;
    static std::unique_ptr<FBO> shadow_map_fbo;

    static FBO::FramebufferAttachment framebuffer_color_attachment_multisampled;
    static FBO::FramebufferAttachment framebuffer_depth_stencil_attachment_multisampled;
    static FBO::FramebufferAttachment framebuffer_color_attachment;
    static FBO::FramebufferAttachment framebuffer_shadow_map_attachment;

    static std::unique_ptr<Mesh<float>> mesh_screen_quad;
    static std::unique_ptr<Material> material_screen_quad;
    static std::unique_ptr<Instance3D> instance_screen_quad;

    static std::unique_ptr<Mesh<float>> mesh_skybox;
    static std::unique_ptr<Material> material_skybox;
    static std::unique_ptr<Instance3D> instance_skybox;

    static std::unique_ptr<Model> model_pig;
    static std::unique_ptr<Material> material_pig;
    static std::unique_ptr<Instance3D> instance_pig;

    static DirectionalLight directional_light(-60.f, 0, 0);

    static std::unique_ptr<VAO> vao_box_gizmo;
    static std::unique_ptr<VAO> vao_axis_gizmo;

    void create_attachments_for_msaa_framebuffer(unsigned int width, unsigned int height) {
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
        framebuffer_color_attachment_multisampled = {
            GL_COLOR_ATTACHMENT0, framebuffer_color_attachment_create_info.target, &framebuffer_color_attachment_texture
        };
        msaa_framebuffer->attach(&framebuffer_color_attachment_multisampled);
        framebuffer_depth_stencil_attachment_multisampled = {
            GL_DEPTH_STENCIL_ATTACHMENT, framebuffer_depth_stencil_attachment_create_info.target, &framebuffer_depth_stencil_attachment_texture
        };
        msaa_framebuffer->attach(&framebuffer_depth_stencil_attachment_multisampled);

        if (GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            plog_error("error -> framebuffer error: {}", std::to_string(status).c_str());

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
        framebuffer_color_attachment = {
            GL_COLOR_ATTACHMENT0, framebuffer_color_attachment_create_info.target, &framebuffer_color_attachment_texture
        };
        intermediate_framebuffer->attach(&framebuffer_color_attachment);

        if (GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            plog_error("error -> framebuffer error: {}", std::to_string(status).c_str());

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

        shadow_map_fbo->bind();

        framebuffer_shadow_map_attachment = {
            GL_DEPTH_ATTACHMENT, framebuffer_shadow_map_attachment_create_info.target, &framebuffer_shadow_map_attachment_texture
        };

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        shadow_map_fbo->attach(&framebuffer_shadow_map_attachment);

        if (GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            plog_error("error -> framebuffer error: {}", std::to_string(status).c_str());

        shadow_map_fbo->unbind();
    }

    Renderer::Renderer(GLFWwindow* window, float width, float height) : width(width), height(height) {
        //IMGUI-INIT
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsClassic();
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding    = 12.0f;
            style.ChildRounding     = 12.0f;
            style.PopupRounding     = 12.0f;
            style.FrameRounding     = 8.0f;
            style.ScrollbarRounding = 12.0f;
            style.GrabRounding      = 8.0f;
            style.FrameBorderSize   = 1.0f;
            style.WindowBorderSize  = 1.0f;
            style.PopupBorderSize   = 1.0f;
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

            mesh_screen_quad = std::make_unique<Mesh<float>>(Geometry::Quad::vertices, Geometry::Quad::indices);
            material_screen_quad = std::make_unique<Material>(&ResourceManager::get_resource<Shader>(SHADER_FRAMEBUFFER), &ResourceManager::get_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2));
            instance_screen_quad = std::make_unique<Instance3D>(
                mesh_screen_quad.get(),
                VAO::AttribInfo {
                    .stride = 5 * sizeof(float),
                    .attribs = {
                        {0, 3, GL_FLOAT, (void*)0},
                        {1, 2, GL_FLOAT, (void*)(3 * sizeof(float))}
                    }
                },
                material_screen_quad.get(),
                glm::vec3(0.f)
            );
        }

        //PHYSICS-INIT
        {
            physics_manager = &Physics::PhysicsManager::get_instance();
            physics_manager->physics_subscribers.push_back(
                [this]() {
                    this->camera->fixed_update();
                }
            );
        }

        //GENERAL-INIT
        {
            camera = &ResourceManager::create_resource<Camera>("camera_game", width, height, glm::vec3(0, 64, 0));

            chunk_manager = std::make_unique<ChunkManager>(camera->position);
            matrices_ubo = std::make_unique<UBO>(0, nullptr, 2 * sizeof(glm::mat4));
            matrices_ubo->bind();
            matrices_ubo->sub_data((void*)glm::value_ptr(camera->get_projection()), sizeof(glm::mat4), 0);
            matrices_ubo->unbind();

            model_pig = std::make_unique<Model>(ASSETS_DIR "models/pig/scene.gltf");
            material_pig = std::make_unique<Material>(&ResourceManager::get_resource<Shader>(SHADER_DEFAULT), model_pig->texture.get());
            instance_pig = std::make_unique<Instance3D>(
                model_pig->meshes[0].get(),
                VAO::AttribInfo {
                    .stride = 8 * sizeof(float),
                    .attribs = {
                        VAO::AttribInfo::Attrib {0, 3, GL_FLOAT, (void*)0},
                        VAO::AttribInfo::Attrib {1, 3, GL_FLOAT, (void*)(3 * sizeof(float))},
                        VAO::AttribInfo::Attrib {2, 2, GL_FLOAT, (void*)(6 * sizeof(float))}
                    }
                },
                material_pig.get(),
                glm::vec3(0.f, 60.f, 0.f)
            );
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
            auto& skybox_texture = ResourceManager::create_resource<Texture>(TEXTURE_SKYBOX_CUBEMAP, skybox_cubemap_texture_create_info);

            mesh_skybox = std::make_unique<Mesh<float>>(Geometry::Cube::vertices, Geometry::Cube::indices_inside);
            material_skybox = std::make_unique<Material>(&ResourceManager::get_resource<Shader>(SHADER_SKYBOX_CUBEMAP), &skybox_texture);
            instance_skybox = std::make_unique<Instance3D>(
                mesh_skybox.get(),
                VAO::AttribInfo {
                    .stride = 3 * sizeof(float),
                    .attribs = {
                        {0, 3, GL_FLOAT, (void *) 0}
                    }
                },
                material_skybox.get(),
                glm::vec3(0.f)
            );
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

        //INSTANCED-RENDERING-INIT
        {}

        //GL-INIT
        {
            glEnable(GL_CULL_FACE);
            if (OPTION_MULTISAMPLING_ENABLED) glEnable(GL_MULTISAMPLE);
            glDisable(GL_BLEND);
            glLineWidth(2.f);
            glViewport(0, 0, width, height);
            glClearColor(.4f, .4f, 1.f, 1.f);
        }

        //MISC-INIT (GIZMO ETC.)
        {
            vao_box_gizmo = std::make_unique<VAO>();
            vao_axis_gizmo = std::make_unique<VAO>();
            Gizmo::setup_line_box_gizmo(vao_box_gizmo.get());
            Gizmo::setup_axis_gizmo(vao_axis_gizmo.get());
        }

        //SUPPORTED-EXTENSIONS
        // #define VOXEL_ENGINE_PRINT_DEBUG
        {
            #ifdef VOXEL_ENGINE_PRINT_DEBUG
            int num_extensions;
            glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

            LOG("==============EXTENSIONS==============");
            for (int i {0}; i < num_extensions; i++) {
                const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
                LOG("{}", extension);
            }
            LOG("======================================");
            #endif
        }
    }

    void Renderer::update(float delta_time) {
        physics_manager ->update();
        chunk_manager   ->update(camera->position);
        camera          ->update(delta_time);
        directional_light.update(camera);

        if (Input::is_key_pressed(GLFW_KEY_X)) debug = !debug;
        if (Input::is_key_pressed(GLFW_KEY_R)) {
            for (auto& shader : ResourceManager::get_storage<Shader>()) {
                shader.second->reload();
                plog_error("{} reloaded ...", shader.first);
            }
        }
    }

    void Renderer::draw_imgui_stuff() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowBgAlpha(0.3f);

        ImGui::Begin("miscellaneous");
        {
            if (ImGui::CollapsingHeader("information", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text((std::to_string(1./Time::delta_time) + std::string(" fps (") + std::to_string(Time::delta_time * 1000.) + std::string(" ms)")).c_str());
                ImGui::Text(std::format("cam: x={:.2f}; y={:.2f}; z={:.2f}", camera->position.x, camera->position.y, camera->position.z).c_str());
            }
            if (ImGui::CollapsingHeader("physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("physics powered by jolt-physics");
            }
            if (ImGui::CollapsingHeader("textures")) {
                static std::string current_item = TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT;
                static std::unordered_map<std::string, void*> framebuffer_textures {
                    { TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2, (void*)(intptr_t)(ResourceManager::get_resource<Texture>(TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2).get_id()) },
                    { TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT, (void*)(intptr_t)(ResourceManager::get_resource<Texture>(TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT).get_id()) },
                };

                if (ImGui::BeginCombo("##combo", current_item.c_str())) {
                    for (auto& [str, id] : framebuffer_textures) {
                        bool is_selected = (current_item == str);
                        if (ImGui::Selectable(str.c_str(), is_selected)) current_item = str;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::Image(framebuffer_textures[current_item], ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
            }
            if (ImGui::CollapsingHeader("lights")) {}
            if (ImGui::CollapsingHeader("chunk-system", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("show_gizmos", &Gizmo::show_gizmos);
                ImGui::Text(
                    std::format(
                        "{} MB/Chunk\n"
                        "{} MB/ChunkCompound\n"
                        "memory: {} MB",
                        (sizeof(Chunk)/1000000.f),
                        (sizeof(ChunkCompound)/1000000.f),
                        (
                            (ChunkManager::num_chunks * NUM_CHUNKS_PER_COMPOUND * sizeof(Chunk)) +
                            (ChunkManager::num_chunks * sizeof(ChunkCompound))
                        ) / 1000000.f
                    ).c_str()
                );
            }
        }
        ImGui::End();

        ImGui::Render();
    }

    void Renderer::render() {
        //SHADOW-RENDER-PASS
        {
            glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            shadow_map_fbo->bind();

            matrices_ubo->bind();
            matrices_ubo->sub_data((void*)glm::value_ptr(directional_light.shadow_map_projection_matrix), sizeof(glm::mat4), 0);
            matrices_ubo->sub_data((void*)glm::value_ptr(directional_light.shadow_map_view_matrix), sizeof(glm::mat4), sizeof(glm::mat4));
            matrices_ubo->unbind();

            glClear(GL_DEPTH_BUFFER_BIT);
            {
                chunk_manager->render_chunk_compounds(directional_light.frustum, ResourceManager::get_resource<Shader>(SHADER_GREEDY_MESH_FOR_SHADOW_PASS));
            }
            shadow_map_fbo->unbind();
        }

        //SCENE-RENDER-PASS
        {
            glViewport(0, 0, width, height);
            msaa_framebuffer->bind();

            matrices_ubo->bind();
            matrices_ubo->sub_data((void*)glm::value_ptr(camera->get_projection()), sizeof(glm::mat4), 0);
            matrices_ubo->sub_data((void*)glm::value_ptr(camera->get_matrix()), sizeof(glm::mat4), sizeof(glm::mat4));
            matrices_ubo->unbind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            {
                glActiveTexture(GL_TEXTURE1);
                std::get<Texture*>(shadow_map_fbo->attachments[0]->attachment_buffer)->bind();
                auto& shader_greedy = ResourceManager::get_resource<Shader>(SHADER_GREEDY_MESH)
                    .use()
                    .set_uniform_int("shadow_map", 1)
                    .set_uniform_mat4("light_space_matrix", directional_light.get_light_space_matrix())
                    .set_uniform_vec3("light_direction", directional_light.direction);
                glActiveTexture(GL_TEXTURE0);
                chunk_manager->render_chunk_compounds(camera->frustum, shader_greedy);
                instance_pig->render();
            }

            //DRAW-SKYBOX
            {
                glDepthFunc(GL_LEQUAL);
                ResourceManager::get_resource<Shader>(SHADER_SKYBOX_CUBEMAP)
                    .use()
                    .set_uniform_mat4("view_non_translated", glm::mat4(glm::mat3(camera->get_matrix())));
                instance_skybox->render();

                glDepthFunc(GL_ALWAYS);
                if (debug) Gizmo::render_axis_gizmo(*camera);
                glDepthFunc(GL_LESS);
            }

            msaa_framebuffer->unbind();
        }

        //FRAMEBUFFER-BLITING
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_framebuffer->get_id());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediate_framebuffer->get_id());
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        //FRAMEBUFFER-PASS
        {
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);
            instance_screen_quad->render();
            draw_imgui_stuff();
            glEnable(GL_DEPTH_TEST);
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
