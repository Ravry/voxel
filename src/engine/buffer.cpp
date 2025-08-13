#include "buffer.h"

#include <iostream>

namespace Voxel {
    VBO::VBO() {
        glGenBuffers(1, &id);
    }

    VBO::~VBO() {
        // std::cout << "deleting vbo" << std::endl;
        glDeleteBuffers(1, &id);
    }

    void VBO::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }
    
    void VBO::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VBO::data(float *data, size_t data_size, GLenum usage)
    {
        glBufferData(GL_ARRAY_BUFFER, data_size, data, usage);
    }

    void VBO::mapped_data(void* data, size_t data_size)
    {
        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_ARRAY_BUFFER, data_size, data, flags);
        buffer_memory_ptr = (void*)glMapBufferRange(GL_ARRAY_BUFFER, 0, data_size, flags);
    }

    void VBO::sub_data(void* data, size_t data_size) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, data);
    }



    EBO::EBO() {
        glGenBuffers(1, &id);
    }

    EBO::~EBO() {
        // std::cout << "deleting ebo" << std::endl;
        glDeleteBuffers(1, &id);
    }

    void EBO::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void EBO::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void EBO::data(void* data, size_t data_size, GLenum usage) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size, data, usage);
    }

    void EBO::mapped_data(void* data, size_t data_size) {
        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, data_size, data, flags);
        buffer_memory_ptr = (void*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, data_size, flags);
    }

    void EBO::sub_data(void* data, size_t data_size) {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, data_size, data);
    }

    VAO::VAO() {
        glGenVertexArrays(1, &id);
    }

    VAO::~VAO() {
        // std::cout << "deleting vao" << std::endl;
        glDeleteVertexArrays(1, &id);
    }

    void VAO::bind() const {
        glBindVertexArray(id);
    }

    void VAO::unbind() const {
        glBindVertexArray(0);
    }
    
    void VAO::attrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
    {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        glEnableVertexAttribArray(index);
    }

    void VAO::attribi(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) {
        glVertexAttribIPointer(index, size, type, stride, pointer);
        glEnableVertexAttribArray(index);
    }




    SSBO::SSBO() {
        glGenBuffers(1, &id);
    }

    SSBO::~SSBO() {
        // std::cout << "deleting ssbo" << std::endl;
        glDeleteBuffers(1, &id);
    }

    void SSBO::bind() const {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, id);
    }

    void SSBO::unbind() const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void SSBO::data(unsigned int index, unsigned int *data, size_t data_size) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, data_size, data, GL_STATIC_DRAW);
    }
}
