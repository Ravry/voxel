#pragma once
#include <print>
#include <format>

#ifdef _MSC_VER
#define LOG(fmt, ...) do { \
std::print("[{}:{}][INFO] ", __FILE__, __LINE__); \
std::print(fmt __VA_OPT__(,) __VA_ARGS__); \
std::print("\n"); \
std::fflush(stdout); \
} while(0)
#else
#define LOG(fmt, ...) do { \
    std::print("[{}:{}][INFO] ", __FILE_NAME__, __LINE__); \
    std::print(fmt __VA_OPT__(,) __VA_ARGS__); \
    std::print("\n"); \
    std::fflush(stdout); \
} while(0)
#endif