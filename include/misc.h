#pragma once

namespace Voxel::Game {
    enum BlockType : unsigned int {
        Bedrock = 0b0,
        Dirt = 0b10,
        Grass = 0b101,
        Air = 0b111,
        MaxValue = Air
    };
}