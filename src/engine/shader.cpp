#include "engine/shader.h"

namespace Voxel {
    void check_status(unsigned int shader, GLenum pname) {
        int success;
        glGetShaderiv(shader, pname, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, NULL, info_log);
            LOG("ERROR::SHADER::COMPILATION_FAILED");
        }
    }

    void Shader::load() {
        id = glCreateProgram();

        std::vector<unsigned int> shaders;
        for (auto &[slot, file]: shader_files) {
            std::string shader_source_str;

            if (!std::filesystem::exists(file)) {
                throw std::runtime_error(std::string("Vertex shader file not found: ") + file.data());
            }

            std::ifstream shader_file;
            shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try {
                shader_file.open(file.data());
                std::stringstream ShaderStream;
                ShaderStream << shader_file.rdbuf();
                shader_file.close();
                shader_source_str = ShaderStream.str();

                const char *shader_source = shader_source_str.c_str();

                unsigned int shader = glCreateShader(slot);
                glShaderSource(shader, 1, &shader_source, 0);
                glCompileShader(shader);
                check_status(shader, GL_COMPILE_STATUS);

                glAttachShader(id, shader);

                shaders.push_back(shader);
            } catch (std::ios_base::failure &e) {
                LOG("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ");
                return;
            }
        }

        glLinkProgram(id);
        GLint success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(id, 1024, nullptr, infoLog);
            LOG("SHADER LINK ERROR: {}", infoLog);
        }

        for (auto &shader: shaders) {
            glDeleteShader(shader);
        }
    }

    Shader::Shader(const std::unordered_map<unsigned int, std::string_view> files) : shader_files(files) {
        load();
    }

    Shader::~Shader() {
        // LOG("Shader::~Shader()");
        glDeleteProgram(id);
    }
    
    Shader& Shader::use() {
        glUseProgram(id);
        return *this;
    }

    void Shader::unuse() {
        glUseProgram(0);
    }


    void Shader::reload() {
        glDeleteProgram(id);
        load();
    }
    
    Shader& Shader::set_uniform_mat4(std::string_view name, glm::mat4 matrix)
    {
        int location = glGetUniformLocation(id, name.data());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
        return *this;
    }

    Shader& Shader::set_uniform_vec3(std::string_view name, glm::vec3 vector) {
        int location = glGetUniformLocation(id, name.data());
        glUniform3fv(location, 1, &vector[0]);
        return *this;
    }

    Shader& Shader::set_uniform_int(std::string_view name, int value) {
        int location = glGetUniformLocation(id, name.data());
        glUniform1i(location, value);
        return *this;
    }
}
