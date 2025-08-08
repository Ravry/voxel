#pragma once
#include <string_view>
#include <map>
#include <memory>
#include <glad/glad.h>
#include "transform.h"
#include "utils.h"

namespace Voxel {
    class Shader {
        private:
            unsigned int id;
            std::string_view vert_file;
            std::string_view frag_file;

        public:
            static std::shared_ptr<Shader> create_shader(std::string name, std::string_view vertex_shader_file, std::string_view fragment_shader_file);
            static std::shared_ptr<Shader> get_shader(std::string name);

            Shader() = default;
            Shader(std::string_view vertex_shader_file, std::string_view fragment_shader_file);
            Shader* use();
            void unuse();
            void destroy();
            void reload();

            Shader* set_uniform_mat4(std::string_view name, glm::mat4 matrix);
            Shader* set_uniform_vec3(std::string_view name, glm::vec3 vector);
            Shader* set_uniform_int(std::string_view name, int value);

        private:
            void load(std::string_view vertex_shader_file, std::string_view fragment_shader_file);
    };
}