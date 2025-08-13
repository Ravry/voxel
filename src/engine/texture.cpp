#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "texture.h"

namespace Voxel {
    Texture::Texture(const TextureCreateInfo& create_info) : target(create_info.target) {
        glGenTextures(1, &id);
        bind();

        glTexParameteri(target, GL_TEXTURE_WRAP_S, create_info.wrap);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, create_info.wrap);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, create_info.min_filter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, create_info.mag_filter);

        switch (target) {
            case GL_TEXTURE_2D_ARRAY: {
                glTexImage3D(
                    target,
                    0,
                    GL_RGBA,
                    create_info.width, create_info.height, create_info.num_textures,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    nullptr
                );

                for (auto& layer_path : create_info.layer_path_map) {
                    unsigned int layer_offset {0};
                    for (auto& path : layer_path.second) {
                        int w, h, channels;
                        unsigned char* data = stbi_load(path.data(), &w, &h, &channels, 4);

                        glTexSubImage3D(
                            target,
                            0,
                            0, 0, layer_path.first + layer_offset,
                            w, h, 1,
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            data
                        );

                        stbi_image_free(data);
                        layer_offset++;
                    }
                }

                glGenerateMipmap(target);
                break;
            }
            case GL_TEXTURE_2D: {
                if (!create_info.data_buffer) {
                    if (create_info.file_path != nullptr) {
                        int w, h, channels;
                        unsigned char* data = stbi_load(create_info.file_path, &w, &h, &channels, 4);

                        if (!data) {
                            is_valid = false;
                        }

                        stbi_image_free(data);
                        break;
                    }
                    else {
                        glTexImage2D(target, 0, create_info.internal_format, create_info.width, create_info.height, 0, create_info.format, create_info.type, nullptr);
                        break;
                    }
                }

                glTexImage2D(target, 0, GL_RED, create_info.width, create_info.height, 0, GL_RED, GL_UNSIGNED_BYTE, create_info.data_buffer);
                glGenerateMipmap(target);
                break;
            }
        }
        unbind();
    }

    void Texture::bind() {
        glBindTexture(target, id);
    }

    void Texture::unbind() {
        glBindTexture(target, 0);
    }

    void Texture::destroy() {
        glDeleteTextures(1, &id);
    }
}
