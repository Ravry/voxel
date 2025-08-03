#pragma once
#include <string_view>
#include "glad/glad.h"
#include "stb_image.h"

namespace Voxel {
    class Texture {
    public:
        Texture(std::string_view path);
        void bind();
        void unbind();
        void destroy();
    private:
        unsigned int id;
    };
}
