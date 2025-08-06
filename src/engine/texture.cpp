#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "texture.h"

namespace Voxel {
    Texture::Texture(const TextureCreateInfo& create_info) : target(create_info.target) {
        glGenTextures(1, &id);
        bind();

        switch (target) {
            case GL_TEXTURE_2D_ARRAY: {
                glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                glTexImage3D(
                    target,
                    0,
                    GL_RGBA,
                    create_info.width, create_info.height, create_info.layer_path_map.size(),
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    nullptr
                );

                for (auto& layer_path : create_info.layer_path_map) {
                    int w, h, channels;
                    unsigned char* data = stbi_load(layer_path.second.data(), &w, &h, &channels, 4);

                    glTexSubImage3D(
                        target,
                        0,
                        0, 0, layer_path.first,
                        w, h, 1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        data
                    );

                    stbi_image_free(data);
                }

                glGenerateMipmap(target);
                break;
            }
            case GL_TEXTURE_2D: {
                glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                int w, h, channels;
                unsigned char* data = stbi_load(create_info.file_path, &w, &h, &channels, 4);

                if (!data) {
                    is_valid = false;
                }

                stbi_image_free(data);
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
