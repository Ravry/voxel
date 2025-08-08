#pragma once
#include <cstdint>
#include "FastNoiseLite.h"

namespace Voxel {
    class Noise {
    public:
        Noise();
        float fetch(float x, float y, float z);
    private:
        FastNoiseLite noise;
    };
}