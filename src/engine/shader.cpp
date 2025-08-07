#include "shader.h"

#include <filesystem>

namespace Voxel {

    void check_status(unsigned int shader, GLenum pname) {
        int success;
        glGetShaderiv(shader, pname, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << std::endl;
        }
    }

    void Shader::load(std::string_view vertex_shader_file, std::string_view fragment_shader_file) {
        std::string vertex_shader_source_str, fragment_shader_source_str;
        Utils::read_file_content(vertex_shader_file, vertex_shader_source_str);
        Utils::read_file_content(fragment_shader_file, fragment_shader_source_str);
        
        const char* vertex_shader_source = vertex_shader_source_str.c_str();
        const char* fragment_shader_source = fragment_shader_source_str.c_str();

        unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, 0);
        glCompileShader(vertex_shader);
        check_status(vertex_shader, GL_COMPILE_STATUS);

        unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, 0);
        glCompileShader(fragment_shader);
        check_status(fragment_shader, GL_COMPILE_STATUS);

        id = glCreateProgram();
        glAttachShader(id, vertex_shader);
        glAttachShader(id, fragment_shader);
        glLinkProgram(id);
        check_status(id, GL_LINK_STATUS);

        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);
    }

    static std::map<std::string, std::shared_ptr<Shader>> shaders;

    std::shared_ptr<Shader> Shader::create_shader(std::string name, std::string_view vertex_shader_file, std::string_view fragment_shader_file) {
        std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertex_shader_file, fragment_shader_file);
        shaders[name] = shader;
        return shader;
    }

    std::shared_ptr<Shader> Shader::get_shader(std::string name) {
        return shaders[name];
    }

    Shader::Shader(std::string_view vertex_shader_file, std::string_view fragment_shader_file) : vert_file(vertex_shader_file), frag_file(fragment_shader_file) {
        load(vertex_shader_file, fragment_shader_file);
    }
    
    Shader* Shader::use() {
        glUseProgram(id);
        return this;
    }

    void Shader::unuse() {
        glUseProgram(0);
    }

    void Shader::destroy() {
        glDeleteShader(id);
    }

    void Shader::reload() {
        destroy();
        load(vert_file, frag_file);
    }
    
    Shader* Shader::set_uniform_mat4(std::string_view name, glm::mat4 matrix)
    {
        int location = glGetUniformLocation(id, name.data());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
        return this;
    }

    Shader* Shader::set_uniform_vec3(std::string_view name, glm::vec3 vector)
    {
        int location = glGetUniformLocation(id, name.data());
        glUniform3fv(location, 1, &vector[0]);
        return this;
    }
}
