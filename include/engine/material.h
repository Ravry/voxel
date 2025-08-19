#pragma once
#include "engine/shader.h"
#include "engine/texture.h"

namespace Voxel {
    class Material {
    public:
        Material(Shader* shader, Texture* texture);
    public:
        Shader* shader;
        Texture* texture;
    };
}