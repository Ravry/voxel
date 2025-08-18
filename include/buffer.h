#pragma once
#include <variant>
#include "glad/glad.h"
#include "texture.h"

namespace Voxel {
    class Buffer {
        protected:
            unsigned int id;
        public:
            virtual void bind() const = 0;
            virtual void unbind() const = 0;
            unsigned int get_id() const { return id; }
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
            struct AttribInfo {
                struct Attrib {
                    GLuint index;
                    GLint size;
                    GLenum type;
                    const void *pointer;
                };

                GLsizei stride;
                std::vector<Attrib> attribs;
            };

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


    class RBO: public Buffer {
    public:
        RBO();
        ~RBO();
        void bind() const override;
        void unbind() const override;
        void storage(GLenum internal_format, GLsizei samples, unsigned int width, unsigned int height);
    };


    class FBO : public Buffer {
    private:
    public:
        struct FramebufferAttachment {
            GLenum attachment;
            GLenum target;
            std::variant<Texture*, RBO*> attachment_buffer;
        };

        FBO();
        ~FBO();
        void bind() const override;
        void unbind() const override;
        void attach(FramebufferAttachment* attachment);
        void refactor(int width, int height);
        
        std::vector<FramebufferAttachment*> attachments;
    };

    class UBO : public Buffer {
    private:
    public:
        UBO(unsigned int binding_point, void *data, size_t size);
        ~UBO();
        void bind() const override;
        void unbind() const override;
        void sub_data(void* data, size_t size, long long offset);
    };
}