#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

#include <iostream>
#include <bits/ostream.tcc>

namespace Voxel {
    Texture::Texture(TextureCreateInfo& create_info) : target(create_info.target) {
        glGenTextures(1, &id);
        bind();

        switch (target) {
            case GL_TEXTURE_2D_ARRAY: {
                std::vector<unsigned char*> texture_datas;
                int width {0}, height {0};
                for (auto path : create_info.paths) {
                    int h, w, channels;
                    unsigned char* data = stbi_load(path.data(), &w, &h, &channels, 4);
                    texture_datas.push_back(data);

                    if (width == 0 && height == 0) {
                        width = w;
                        height = h;
                    }
                    else if (width != w || height != h) {
                        std::cout << "TEXTURE::GL_TEXTURE_2D_ARRAY ERROR - not all textures share the same dimensions" << std::endl;
                    }
                }

                glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                int layer_count = texture_datas.size();
                glTexImage3D(
                    target,
                    0,
                    GL_RGBA,
                    width, height, layer_count,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    nullptr
                );

                for (int i = 0; i < layer_count; i++) {
                    glTexSubImage3D(
                        target,
                        0,
                        0, 0, i,
                        width, height, 1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        texture_datas[i]
                    );
                }

                for (auto data : texture_datas) {
                    stbi_image_free(data);
                }

                glGenerateMipmap(target);
                break;
            }
            case GL_TEXTURE_3D: {
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                glTexImage3D(target, 0, GL_R8, 8, 8, 8, 0, GL_RED, GL_UNSIGNED_BYTE, create_info.data_buffer);
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
