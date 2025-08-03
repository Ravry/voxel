#include "buffer.h"

namespace Voxel {
    void Buffer::destroy() {
        glDeleteBuffers(1, &id);
    }

    VBO::VBO() {
        glGenBuffers(1, &id);
    }

    void VBO::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }
    
    void VBO::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VBO::data(float *data, size_t data_size)
    {
        glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
    }

    void VBO::mapped_data(float* data, size_t data_size)
    {
        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_ARRAY_BUFFER, data_size, data, flags);
        buffer_memory_ptr = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0, data_size, flags);
    }

    EBO::EBO() {
        glGenBuffers(1, &id);
    }

    void EBO::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void EBO::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void EBO::data(unsigned int *data, size_t data_size)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
    }

    VAO::VAO() {
        glGenVertexArrays(1, &id);
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
}