#pragma once
#include <string_view>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include "glad/glad.h"
#include "stb_image.h"

namespace Voxel {
    class Texture {
    public:
        struct TextureCreateInfo {
            GLenum target;
            GLenum internal_format;
            GLenum format;
            GLenum type;
            const char* file_path {nullptr};
            unsigned int width;
            unsigned int height;
            GLint wrap {GL_REPEAT};
            GLint min_filter {GL_NEAREST_MIPMAP_NEAREST};
            GLint mag_filter {GL_NEAREST};
            std::map<unsigned int, std::vector<std::string_view>> layer_path_map;
            unsigned int num_textures;
            void* data_buffer {nullptr};
            GLsizei samples;
        };

        static std::shared_ptr<Texture> fallback() {
            static std::shared_ptr<Texture> fallback_texture = std::make_shared<Texture>(TextureCreateInfo {
                .target = GL_TEXTURE_2D,
                .file_path = ASSETS_DIR "textures/double_checkered.png",
            });
            return fallback_texture;
        }

        Texture(const TextureCreateInfo& create_info);
        ~Texture();
        void bind();
        void unbind();

        unsigned int get_id() { return id; }

    private:
        unsigned int id;
        GLenum target;

    public:
        bool is_valid {true};
    };
}
