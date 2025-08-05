#pragma once
#include <iostream>
#include <string_view>
#include <string>
#include <fstream>
#include <sstream>

namespace Voxel {
    namespace Utils {
        static void read_file_content(std::string_view file_name, std::string& file_content) {
            std::ifstream file(file_name.data());
            if (!file.is_open()) {
                std::cerr << "failed to open file " << file_name << "\n";
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file_content = buffer.str();
        }
    }

    namespace Time {
        class Timer {
        public:
            static double delta_time;
        };
    }
}