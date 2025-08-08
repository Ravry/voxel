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
            const char* file_path;
            unsigned int width;
            unsigned int height;
            std::map<unsigned int, std::vector<std::string_view>> layer_path_map;
            unsigned int num_textures;
            void* data_buffer;
        };

        static std::shared_ptr<Texture> fallback() {
            static std::shared_ptr<Texture> fallback_texture = std::make_shared<Texture>(TextureCreateInfo {
                .target = GL_TEXTURE_2D,
                .file_path = ASSETS_DIR "textures/double_checkered.png",
            });
            return fallback_texture;
        }

        static std::shared_ptr<Texture> create_texture_or_fallback(const TextureCreateInfo& create_info) {
            std::shared_ptr<Texture> texture = std::make_shared<Texture>(create_info);
            if (!texture->is_valid) {
                std::cout << "[WARNING] " << " the texture couldn't be created (invalid texture)!" << std::endl;
                return fallback();
            }

            return texture;
        }

        Texture(const TextureCreateInfo& create_info);
        void bind();
        void unbind();
        void destroy();

    private:
        unsigned int id;
        GLenum target;

    public:
        bool is_valid {true};
    };
}
