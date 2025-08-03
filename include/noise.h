#pragma once
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include "utils.h"

namespace Voxel {
    class Noise {
    public:
        Noise();
        int fetch(uint8_t x, uint8_t y, uint8_t z);
    };
}
