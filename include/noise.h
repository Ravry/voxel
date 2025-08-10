#pragma once
#include <cstdint>
#include "FastNoiseLite.h"

namespace Voxel {
    class Noise {
    public:
        Noise();
        float fetch_heightmap(float x, float z);
        float fetch_cave(float x, float y, float z);
    private:
        FastNoiseLite terrain_noise;
        FastNoiseLite biome_noise;
    };
}