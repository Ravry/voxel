#pragma once
#include "glad/glad.h"

namespace Voxel {
    class Buffer {
        protected:
            unsigned int id;
        public:
            virtual void bind() const = 0;
            virtual void unbind() const = 0;
    };

    class VBO : public Buffer {
        public:
            VBO();
            ~VBO();
            void bind() const override;
            void unbind() const override;
            void data(float* data, size_t data_size, GLenum usage);
            void mapped_data(void* data, size_t data_size);
            void sub_data(void* data, size_t data_size);
            void* buffer_memory_ptr;
    };

    class EBO : public Buffer {
        public:
            EBO();
            ~EBO();
            void bind() const override;
            void unbind() const override;
            void data(void* data, size_t data_size, GLenum usage);
            void mapped_data(void* data, size_t data_size);
            void sub_data(void* data, size_t data_size);
            void* buffer_memory_ptr;
    };

    class VAO : public Buffer {
        public:
            VAO();
            ~VAO();
            void bind() const override;
            void unbind() const override;
            void attrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
            void attribi(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
    };

    class SSBO : public Buffer {
    public:
        SSBO();
        ~SSBO();
        void bind() const override;
        void unbind() const override;
        void data(unsigned int index, unsigned int *data, size_t data_size);
    };
}