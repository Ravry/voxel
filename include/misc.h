#pragma once
#include <stdint.h>

namespace Voxel::Game {
    //TEXTURES
    #define TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT "texture_framebuffer_color_attachment"
    #define TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2 "texture_framebuffer_color_attachment2"
    #define TEXTURE_FRAMEBUFFER_DEPTH_STENCIL_ATTACHMENT "texture_framebuffer_depth_stencil_attachment"

    //SHADERS
    #define SHADER_DEFAULT "shader_default"
    #define SHADER_FRAMEBUFFER "shader_framebuffer"
    #define SHADER_GREEDY_MESH "shader_greedy_mesh"

    //OPTIONS
    #define OPTION_MULTISAMPLING_ENABLED true

    constexpr int SIZE = sizeof(uint16_t) * 8;

    //NOTE: LSB is indicator for multi-image-block-types
    enum BlockType : unsigned int {
        Air = 0b0,
        Dirt = 0b10,
        Stone = 0b100,
        Snow = 0b110,
        Grass = 0b1001,
        Wood = 0b1111,
        Leafs = 0b10100,
        Diamond = 0b10110,
        Bedrock = 0b11000,
        MaxValue = Bedrock
    };

    enum BiomeType : uint8_t {
        Plains = 0,
        Mountains = 1,
    };

    struct Biome {
        float max_amplitude;
        int min_height;
    };
}