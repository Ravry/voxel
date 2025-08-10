#pragma once

namespace Voxel::Game {
    enum BlockType : unsigned int {
        Bedrock = 0b0,
        Dirt = 0b10,
        Stone = 0b100,
        Snow = 0b110,
        Grass = 0b1001,
        Wood = 0b1111,
        Leafs = 0b10100,
        Air = 0b111,
        MaxValue = Air
    };
}