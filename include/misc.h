#pragma once

namespace Voxel {
    namespace Game {
        enum BlockType : unsigned int {
            Bedrock = 0,
            Dirt = 1,
            Grass = 2,
            Air = 3,
            MaxValue = Air
        };
    }
}