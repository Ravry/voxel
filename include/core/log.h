#pragma once
#include <print>
#include <format>

constexpr const char* COLOR_RESET   = "\033[0m";
constexpr const char* COLOR_RED     = "\033[31m";
constexpr const char* COLOR_YELLOW  = "\033[33m";
constexpr const char* COLOR_GREEN   = "\033[32m";

#define CONSOLE_HINT_INFO "INFO"
#define CONSOLE_HINT_WARNING "WARNING"
#define CONSOLE_HINT_ERROR "ERROR"

template <typename... Args>
void log_impl(const char* color, const char* file, int line, const char* hint, std::format_string<Args...> fmt, Args&&... args) {
    std::print("[{}:{}][{}{}{}] ", file, line, color, hint, COLOR_RESET);
    std::print(fmt, std::forward<Args>(args)...);
    std::print("\n");
    std::fflush(stdout);
}

#define plog(fmt, ...) log_impl(COLOR_GREEN, __FILE_NAME__, __LINE__, CONSOLE_HINT_INFO, fmt __VA_OPT__(,) __VA_ARGS__)
#define plog_warn(fmt, ...) log_impl(COLOR_YELLOW, __FILE_NAME__, __LINE__, CONSOLE_HINT_WARNING,  fmt __VA_OPT__(,) __VA_ARGS__)
#define plog_error(fmt, ...) log_impl(COLOR_RED, __FILE_NAME__, __LINE__, CONSOLE_HINT_ERROR,  fmt __VA_OPT__(,) __VA_ARGS__)