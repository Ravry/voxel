#pragma once

namespace Voxel::Game {
    //NOTE: LSB is indicator for multi-image-block-types
    enum BlockType : unsigned int {
        Air = 0b0,
        Dirt = 0b10,
        Stone = 0b100,
        Snow = 0b110,
        Grass = 0b1001,
        Wood = 0b1111,
        Leafs = 0b10100,
        Bedrock = 0b10110,
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