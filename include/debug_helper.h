#pragma once
#include <chrono>

namespace Voxel::Debug {
    static std::chrono::high_resolution_clock::time_point start_timer() {
        return std::chrono::high_resolution_clock::now();
    }

    static double stop_timer(std::chrono::high_resolution_clock::time_point start, std::string msg = "operation") {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        double milliseconds = duration.count();
        return milliseconds;
    }
}
