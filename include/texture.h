#pragma once
#include <string_view>
#include <vector>
#include "glad/glad.h"
#include "stb_image.h"

namespace Voxel {
    class Texture {
    public:
        struct TextureCreateInfo {
            GLenum target;
            std::vector<std::string_view> paths;
            void* data_buffer;
        };

        Texture(TextureCreateInfo& create_info);
        void bind();
        void unbind();
        void destroy();
    private:
        unsigned int id;
        GLenum target;
    };
}
