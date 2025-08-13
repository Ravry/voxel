#pragma once
#include <print>

#define LOG(...) do { \
        std::print("[{}:{}][INFO] ", __FILE_NAME__, __LINE__); \
        std::print(__VA_ARGS__); \
        std::print("\n"); \
        std::fflush(stdout); \
    } while(0)
