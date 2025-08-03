#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

namespace Voxel {
    Texture::Texture(std::string_view path) {
        glGenTextures(1, &id);
        bind();

        int width, height, channels;
        unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 4);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage3D(
            GL_TEXTURE_2D_ARRAY,
            0,
            GL_RGBA,
            width, height, 1,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        unbind();
    }

    void Texture::bind() {
        glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    }

    void Texture::unbind() {
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    void Texture::destroy() {
        glDeleteTextures(1, &id);
    }
}