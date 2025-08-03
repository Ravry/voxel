#include "instance3D.h"

#include <bitset>
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


        auto start = std::chrono::high_resolution_clock::now();

        #pragma region voxel_axis_setup
        uint8_t voxels_zy_top_face[size * size] = {};
        uint8_t voxels_zy_bottom_face[size * size] = {};

        uint8_t voxels_xz_right_face[size * size] = {};
        uint8_t voxels_xz_left_face[size * size] = {};

        uint8_t voxels_xz_front_face[size * size] = {};
        uint8_t voxels_xz_back_face[size * size] = {};

        for (size_t x {0}; x < size; x++) {
            for (size_t z {0}; z < size; z++) {
                //x-z -> uint8_t
                uint8_t row_vertical = voxels[x + (z * size) + (size * size * 2)];

                uint8_t row_top_face = row_vertical & ~(row_vertical >> 1);
                uint8_t row_bottom_face = row_vertical & ~(row_vertical << 1);

                for (uint8_t y {0}; y < size; y++) {
                    voxels_zy_top_face[z + (y * size)] |= ((row_top_face >> y) & 1) << x;
                    voxels_zy_bottom_face[z + (y * size)] |= ((row_bottom_face >> y) & 1) << x;
                }
            }
        }

        for (size_t y {0}; y < size; y++) {
            for (size_t x {0}; x < size; x++) {
                //x-y -> uint8_t
                uint8_t row2 = voxels[x + (y * size) + (size * size)];

                uint8_t row_front_face = row2 & ~(row2 >> 1);
                uint8_t row_back_face = row2 & ~(row2 << 1);

                for (size_t z {0}; z < size; z++) {
                    voxels_xz_front_face[x + (z * size)] |= ((row_front_face >> z) & 1) << y;
                    voxels_xz_back_face[x + (z * size)] |= ((row_back_face >> z) & 1) << y;
                }
            }
        }

        for (size_t y {0}; y < size; y++) {
            for (size_t z {0}; z < size; z++) {
                //z-y -> uint8_t
                uint8_t row = voxels[z + (y * size)];

                uint8_t row_right_face = row & ~(row >> 1);
                uint8_t row_left_face = row & ~(row << 1);

                for (size_t x {0}; x < size; x++) {
                    voxels_xz_right_face[x + (z * size)] |= ((row_right_face >> x) & 1) << y;
                    voxels_xz_left_face[x + (z * size)] |= ((row_left_face >> x) & 1) << y;
                }
            }
        }

        uint8_t** arrays = new uint8_t*[6] {
            voxels_zy_top_face,
            voxels_zy_bottom_face,
            voxels_xz_right_face,
            voxels_xz_left_face,
            voxels_xz_front_face,
            voxels_xz_back_face,
        };
        #pragma endregion



        #pragma region greedy_meshing
        for (size_t y {0}; y < size; y++)
        {
            for (size_t z {0}; z < size; z++)
            {
                for (int i {0}; i < 2; i++)
                {
                    uint8_t& row = arrays[i][z + (y * size)];

                    if (row == 0)
                        continue;

                    while (row != 0) {
                        uint8_t x0 = std::__countr_zero(row);
                        uint8_t height = std::__countr_zero(~(row >> x0)); //x-direction this time
                        uint8_t mask = ((1u << height) - 1) << x0;

                        uint8_t width {1};
                        row ^= mask;

                        for (size_t z0 = (z + 1); z0 < size; z0++)
                        {
                            if ((mask & arrays[i][z0 + (y * size)]) != mask)
                                break;

                            arrays[i][z0 + (y * size)] ^= mask;
                            width++;
                        }

                        size_t old_v_size = vertices.size();
                        size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 12);
                        indices.resize(old_i_size + 6);

                        float* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        float z0f = (float)z - 0.5f;
                        float z1f = (float)(z + width) - .5f;
                        float x0f = (float)x0 - 0.5f;
                        float x1f = (float)(x0 + height) - 0.5f;
                        float y0f = (float)y + 0.5f;

                        if (i == 1)
                            y0f -= 1.f;

                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x1f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z1f;
                        *vertex_ptr++ = x1f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z1f;

                        *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                        *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;

                        triangles += 4;
                    }
                }
            }
        }

        for (size_t z {0}; z < size; z++)
        {
            for (size_t x {0}; x < size; x++)
            {
                for (int i = 4; i < 6; i++) {
                    uint8_t& row = arrays[i][x + (z * size)];

                    if (row == 0)
                        continue;

                    while (row != 0) {
                        uint8_t y0 = std::__countr_zero(row);
                        uint8_t height = std::__countr_zero(~(row >> y0));
                        uint8_t mask = ((1u << height) - 1) << y0;

                        uint8_t width {1};
                        row ^= mask;

                        for (size_t x0 = (x + 1); x0 < size;x0++)
                        {
                            if ((mask & arrays[i][x0 + (z * size)]) != mask)
                                break;

                            arrays[i][x0 + (z * size)] ^= mask;
                            width++;
                        }

                        size_t old_v_size = vertices.size();
                        size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 12);
                        indices.resize(old_i_size + 6);

                        float* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        float z0f = (float)z - 0.5f;
                        float x0f = (float)x - 0.5f;
                        float x1f = (float)(x + width) - 0.5f;
                        float y0f = (float)y0 - 0.5f;
                        float y1f = (float)(y0 + height) - 0.5f;

                        if (i == 4) {
                            z0f += 1.f;
                        }

                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y1f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x1f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x1f;  *vertex_ptr++ = y1f; *vertex_ptr++ = z0f;

                        *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                        *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;

                        triangles += 4;
                    }
                }
            }
        }

        for (size_t z {0}; z < size; z++)
        {
            for (size_t x {0}; x < size; x++)
            {
                for (int i {2}; i < 4; i++) {
                    uint8_t& row = arrays[i][x + (z * size)];

                    if (row == 0)
                        continue;

                    while (row != 0) {
                        uint8_t y0 = std::__countr_zero(row);
                        uint8_t height = std::__countr_zero(~(row >> y0));
                        uint8_t mask = ((1u << height) - 1) << y0;

                        uint8_t width {1};
                        row ^= mask;

                        for (size_t z0 = (z + 1); z0 < size; z0++)
                        {
                            if ((mask & arrays[i][x + (z0 * size)]) != mask)
                                break;

                            arrays[i][x + (z0 * size)] ^= mask;
                            width++;
                        }

                        size_t old_v_size = vertices.size();
                        size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 12);
                        indices.resize(old_i_size + 6);

                        float* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        float z0f = (float)z - 0.5f;
                        float z1f = (float)(z + width) - 0.5f;
                        float x0f = (float)x + 0.5f;
                        float y0f = (float)y0 - 0.5f;
                        float y1f = (float)(y0 + height) - 0.5f;

                        if (i == 3)
                            x0f -= 1.f;

                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y1f; *vertex_ptr++ = z0f;
                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z1f;
                        *vertex_ptr++ = x0f;  *vertex_ptr++ = y1f; *vertex_ptr++ = z1f;

                        *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                        *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;

                        triangles += 4;
                    }
                }
            }
        }
        #pragma endregion

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "operation took " << duration.count() << " milliseconds\n";

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