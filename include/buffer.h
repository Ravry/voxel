#pragma once
#include "glad/glad.h"

namespace Voxel {
    class Buffer {
        protected:
            unsigned int id;
        public:
            virtual void bind() const = 0;
            virtual void unbind() const = 0;
            virtual void destroy(); 
            virtual ~Buffer() = default;
    };

    class VBO : public Buffer {
        public:
            VBO();
            void bind() const override;
            void unbind() const override;
            void data(float* data, size_t data_size);
            void mapped_data(float* data, size_t data_size);
        private:
            float* buffer_memory_ptr;
    };

    class EBO : public Buffer {
        public:
            EBO();
            void bind() const override;
            void unbind() const override;
            void data(unsigned int* data, size_t data_size);
    };

    class VAO : public Buffer {
        public:
            VAO();
            void bind() const override;
            void unbind() const override;
            void attrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    };
}