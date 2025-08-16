#pragma once
#include <string_view>
#include <memory>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include "log.h"
#include "transform.h"
#include "utils.h"

namespace Voxel {
    class Shader {
        private:
            unsigned int id;
            std::unordered_map<unsigned int, std::string_view> shader_files;
        public:
            Shader() = default;

            Shader(const std::unordered_map<unsigned int, std::string_view> shader_files);
            ~Shader();

            Shader& use();
            void unuse();
            void reload();

            Shader& set_uniform_mat4(std::string_view name, glm::mat4 matrix);
            Shader& set_uniform_vec3(std::string_view name, glm::vec3 vector);
            Shader& set_uniform_int(std::string_view name, int value);

        private:
            void load();
    };
}