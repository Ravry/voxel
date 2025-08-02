#include "instance3D.h"
#include <chrono>

namespace Voxel {
    Mesh::Mesh(PrimitiveType primitive) {
        vao.bind();
        vbo.bind();
        ebo.bind();
        
        switch (primitive) {
            case PrimitiveType::Cube: {
                vbo.data(Geometry::Cube::vertices, sizeof(Geometry::Cube::vertices));
                ebo.data(Geometry::Cube::indices, sizeof(Geometry::Cube::indices));
                triangles = sizeof(Geometry::Cube::indices)/sizeof(unsigned int);
                break;
            }
            default: {
                vbo.data(Geometry::Triangle::vertices, sizeof(Geometry::Triangle::vertices));
                ebo.data(Geometry::Triangle::indices, sizeof(Geometry::Triangle::indices));
                triangles = sizeof(Geometry::Triangle::indices)/sizeof(unsigned int);
                break;
            }
        }
        
        vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    Mesh::Mesh(uint8_t* voxels, size_t size) {
        vao.bind();
        vbo.bind();
        ebo.bind();

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        vertices.reserve(size * size * 8 * 12);
        indices.reserve(size * size * 8 * 6);

        uint8_t voxels_zy_down_face[size * size] = {};

        for (size_t x {0}; x < size; x++) {
            for (size_t z {0}; z < size; z++) {
                uint8_t row = voxels[x + (z * size) + (size * size * 2)];
                row &= ~(row >> 1);

                for (uint8_t y {0}; y < size; y++) {
                    uint8_t value = (row >> y) & 1;
                    voxels_zy_down_face[z + (y * size)] |= value << x;
                }
            }
        }

        for (size_t y {0}; y < size; y++)
        {
            for (size_t z {0}; z < size; z++) 
            {
                uint8_t& row = voxels_zy_down_face[z + (y * size)];
                if (row == 0)
                    continue;
                
                while (row != 0)
                {
                    uint8_t x0 = std::__countr_zero(row);
                    uint8_t width = std::__countr_zero(~(row >> x0));
                    uint8_t mask = ((1u << width) - 1) << x0;
                    
                    uint8_t height {1};
                    row ^= mask;
                    for (size_t z0 = (z + 1); z0 < size; z0++)
                    {
                        if ((mask & voxels_zy_down_face[z0 + (y * size)]) != mask)
                            break;
                        
                        voxels_zy_down_face[z0 + (y * size)] ^= mask;
                        height++;
                    }

                    size_t old_v_size = vertices.size();
                    size_t old_i_size = indices.size();
                    vertices.resize(old_v_size + 12);
                    indices.resize(old_i_size + 6);

                    float* v = vertices.data() + old_v_size;
                    unsigned int* idx = indices.data() + old_i_size;

                    float x0f = (float)x0 - 0.5f;
                    float x1f = (float)(x0 + width) - 0.5f;
                    float z0f = (float)z - 0.5f;
                    float z1f = (float)(z + height) - 0.5f;
                    float y0f = (float)y + 0.5f;

                    *v++ = x0f; *v++ = y0f; *v++ = z0f;
                    *v++ = x1f; *v++ = y0f; *v++ = z0f;
                    *v++ = x0f; *v++ = y0f; *v++ = z1f;
                    *v++ = x1f; *v++ = y0f; *v++ = z1f;

                    *idx++ = triangles + 0; *idx++ = triangles + 1; *idx++ = triangles + 2;
                    *idx++ = triangles + 1; *idx++ = triangles + 3; *idx++ = triangles + 2;

                    triangles += 4;
                }
            }
        }

        triangles = indices.size();
        std::cout << "triangles: " << triangles / 3 << "\n";

        vbo.data(vertices.data(), vertices.size() * sizeof(float));
        ebo.data(indices.data(), indices.size() * sizeof(unsigned int));

        vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    void Mesh::render() {
        vao.bind();
        glDrawElements(GL_TRIANGLES, triangles, GL_UNSIGNED_INT, 0);
        vao.unbind();
    }

    Instance3D::Instance3D(Mesh* model, glm::vec3 albedo, glm::vec3 position, glm::vec3 scale) : Transform(position, glm::vec3(0), scale) {
        this->model = model;
        this->albedo = albedo;
    }

    Instance3D::~Instance3D() { }

    void Instance3D::render(Shader& shader) {
        shader
            .set_uniform_mat4("model", matrix)
            .set_uniform_vec3("albedo", albedo);
        model->render();
    }
}