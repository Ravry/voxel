#pragma once
#include "shader.h"
#include "texture.h"

namespace Voxel {
    class Material {
    public:
        Material(Shader* shader, Texture* texture);
    public:
        Shader* shader;
        Texture* texture;
    };
}