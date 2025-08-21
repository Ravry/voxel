#pragma once
#include <stdint.h>

namespace Voxel::Game {
    //TEXTURES
    #define TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT "texture_framebuffer_color_attachment"
    #define TEXTURE_FRAMEBUFFER_COLOR_ATTACHMENT2 "texture_framebuffer_color_attachment2"
    #define TEXTURE_FRAMEBUFFER_DEPTH_STENCIL_ATTACHMENT "texture_framebuffer_depth_stencil_attachment"
    #define TEXTURE_FRAMEBUFFER_SHADOW_MAP_ATTACHMENT "texture_framebuffer_shadow_map_attachment"
    #define TEXTURE_FRAMEBUFFER_COLOR_MAP_ATTACHMENT "texture_framebuffer_color_map_attachment"
    #define TEXTURE_SKYBOX_CUBEMAP "texture_skybox_cubemap"

    //SHADERS
    #define SHADER_SLOT_VERTEX 0
    #define SHADER_SLOT_FRAGMENT 1
    #define SHADER_SLOT_GEOMETRY 2

    #define SHADER_DEFAULT "shader_default"
    #define SHADER_FRAMEBUFFER "shader_framebuffer"
    #define SHADER_SKYBOX_CUBEMAP "shader_skybox_cubemap"
    #define SHADER_GREEDY_MESH_FOR_SHADOW_PASS "shader_greedy_mesh_shadow_pass"
    #define SHADER_GREEDY_MESH "shader_greedy_mesh"

    //OPTIONS
    #define OPTION_MULTISAMPLING_ENABLED true

    constexpr int SIZE = sizeof(uint16_t) * 8;
    constexpr int SIZE_CUBIC = SIZE * SIZE * SIZE;
    constexpr int SIZE_VALUE_IN_BITS = 8;
    constexpr int NUM_VALUES_IN_ONE_UINT = 4;
    constexpr int NUM_CHUNKS_PER_COMPOUND = 16;

    //NOTE: LSB is indicator for MULTI-IMAGE-BLOCK-TYPES
    enum BlockType : uint8_t {
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
}