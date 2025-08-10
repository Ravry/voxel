#pragma once
#include <cstdint>
#include "FastNoiseLite.h"

namespace Voxel {
    class Noise {
    public:
        Noise();
        float fetch_heightmap(float x, float z);
    private:
        FastNoiseLite noise;
    };
}