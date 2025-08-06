#include "mesh.h"
#include "chunk.h"
#include "debug_helper.h"
#include <stdio.h>
#include <bit>
#include <bitset>

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

    uint32_t packed_vertex_data(
        uint8_t pos_x, uint8_t pos_y, uint8_t pos_z,
        uint8_t flip, uint8_t norm_x, uint8_t norm_y, uint8_t norm_z,
        uint8_t uv_x, uint8_t uv_y
    ) {
        uint32_t packed = 0;
        packed |= (pos_x & 0x1F) << 27;
        packed |= (pos_y & 0x1F) << 22;
        packed |= (pos_z & 0x1F) << 17;

        packed |= (uv_x & 0xF) << 13;
        packed |= (uv_y & 0xF) << 9;

        packed |= (flip & 0x1) << 8;
        packed |= (norm_x & 0x1) << 7;
        packed |= (norm_y & 0x1) << 6;
        packed |= (norm_z & 0x1) << 5;

        return packed;
    }

    Mesh::Mesh(uint16_t* voxels, Game::Chunk** chunks, const std::size_t size)
    {
        vao.bind();
        vbo.bind();
        ebo.bind();

        std::vector<uint32_t> vertices;
        std::vector<unsigned int> indices;

        vertices.reserve(size * size * 4 * 6);
        indices.reserve(size * size * 8 * 6);

        auto start = Debug::start_timer();

        #pragma region face_culling
        uint16_t voxels_zy_top_face[size * size] = {};
        uint16_t voxels_zy_bottom_face[size * size] = {};

        uint16_t voxels_xz_right_face[size * size] = {};
        uint16_t voxels_xz_left_face[size * size] = {};

        uint16_t voxels_xz_front_face[size * size] = {};
        uint16_t voxels_xz_back_face[size * size] = {};

        for (std::size_t i {0}; i < size; i++) {
            for (std::size_t j {0}; j < size; j++) {
                //x-axis
                uint16_t row = voxels[j + (i * size)];
                uint32_t row_neighbors_included = static_cast<uint32_t>(row) << 1;

                uint16_t row_x_minus_one_neighbor = chunks[0] ? chunks[0]->voxels[j + (i * size)] : 0;
                row_neighbors_included |= row_x_minus_one_neighbor >> 15;

                uint16_t row_x_plus_one_neighbor = chunks[1] ? chunks[1]->voxels[j + (i * size)] : 0;
                row_neighbors_included |= (row_x_plus_one_neighbor & 1) << 17;

                uint16_t row_right_face = static_cast<uint16_t>((row_neighbors_included & ~(row_neighbors_included >> 1)) >> 1);
                uint16_t row_left_face = static_cast<uint16_t>((row_neighbors_included & ~(row_neighbors_included << 1)) >> 1);


                //z-axis
                uint16_t row2 = voxels[j + (i * size) + (size * size)];
                uint32_t row2_neighbors_included = static_cast<uint32_t>(row2) << 1;

                uint16_t row_z_minus_one_neighbor = chunks[2] ? chunks[2]->voxels[j + (i * size) + (size * size)] : 0;
                row2_neighbors_included |= row_z_minus_one_neighbor >> 15;

                uint16_t row_z_plus_one_neighbor = chunks[3] ? chunks[3]->voxels[j + (i * size) + (size * size)] : 0;
                row2_neighbors_included |= (row_z_plus_one_neighbor & 1) << 17;

                uint16_t row_front_face = static_cast<uint16_t>((row2_neighbors_included & ~(row2_neighbors_included >> 1)) >> 1);
                uint16_t row_back_face = static_cast<uint16_t>((row2_neighbors_included & ~(row2_neighbors_included << 1)) >> 1);

                //y-axis
                uint16_t row_vertical = voxels[i + (j * size) + (size * size * 2)];
                uint32_t row_vertical_neighbors_included = static_cast<uint32_t>(row_vertical) << 1;

                uint16_t row_y_minus_one_neighbor = chunks[4] ? chunks[4]->voxels[i + (j * size) + (size * size * 2)] : 0;
                row_vertical_neighbors_included |= row_y_minus_one_neighbor >> 15;

                uint16_t row_y_plus_one_neighbor = chunks[5] ? chunks[5]->voxels[i + (j * size) + (size * size * 2)] : 0;
                row_vertical_neighbors_included |= (row_y_plus_one_neighbor & 1) << 17;

                uint16_t row_top_face = static_cast<uint16_t>((row_vertical_neighbors_included & ~(row_vertical_neighbors_included >> 1)) >> 1);
                uint16_t row_bottom_face = static_cast<uint16_t>((row_vertical_neighbors_included & ~(row_vertical_neighbors_included << 1)) >> 1);

                for (std::size_t k {0}; k < size; k++) {
                    voxels_xz_right_face[k + (j * size)] |= ((row_right_face >> k) & 1) << i;
                    voxels_xz_left_face[k + (j * size)] |= ((row_left_face >> k) & 1) << i;

                    voxels_xz_front_face[j + (k * size)] |= ((row_front_face >> k) & 1) << i;
                    voxels_xz_back_face[j + (k * size)] |= ((row_back_face >> k) & 1) << i;

                    voxels_zy_top_face[j + (k * size)] |= ((row_top_face >> k) & 1) << i;
                    voxels_zy_bottom_face[j + (k * size)] |= ((row_bottom_face >> k) & 1) << i;
                }
            }
        }


        uint16_t** arrays = new uint16_t*[6] {
            voxels_zy_top_face,
            voxels_zy_bottom_face,
            voxels_xz_right_face,
            voxels_xz_left_face,
            voxels_xz_front_face,
            voxels_xz_back_face,
        };
        #pragma endregion

        double time_culling = Debug::stop_timer(start);

        #pragma region greedy_meshing
        #pragma region vertical_faces
        for (std::size_t y {0}; y < size; y++)
        {
            for (std::size_t z {0}; z < size; z++)
            {
                for (int i {0}; i < 2; i++)
                {
                    uint16_t& row = arrays[i][z + (y * size)];

                    if (row == 0)
                        continue;

                    while (row != 0) {
                        uint16_t x0 = std::countr_zero(row);
                        uint16_t height = std::countr_zero(static_cast<uint16_t>(~(row >> x0)));
                        uint16_t mask = ((1u << height) - 1) << x0;

                        uint16_t width {1};
                        row ^= mask;

                        for (std::size_t z0 = (z + 1); z0 < size; z0++)
                        {
                            if ((mask & arrays[i][z0 + (y * size)]) != mask)
                                break;

                            arrays[i][z0 + (y * size)] ^= mask;
                            width++;
                        }

                        std::size_t old_v_size = vertices.size();
                        std::size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 4);
                        indices.resize(old_i_size + 6);

                        uint32_t* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        uint8_t z0f = z;
                        uint8_t z1f = (z + width);
                        uint8_t x0f = x0;
                        uint8_t x1f = (x0 + height);
                        uint8_t y0f = y;

                        uint8_t norm_flip = 0;
                        if (i == 0) {
                            y0f += 1.f;
                            norm_flip = 1;
                        }

                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z0f, norm_flip,  0, 1, 0, 0, 0);
                        *vertex_ptr++ = packed_vertex_data(x1f, y0f, z0f, norm_flip,  0, 1, 0, height, 0);
                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z1f, norm_flip,  0, 1, 0, 0, width);
                        *vertex_ptr++ = packed_vertex_data(x1f, y0f, z1f, norm_flip,  0, 1, 0, height, width);

                        *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                        *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;

                        triangles += 4;
                    }
                }
            }
        }
        #pragma endregion

        #pragma region left_and_right_faces
        for (std::size_t z {0}; z < size; z++)
        {
            for (std::size_t x {0}; x < size; x++)
            {
                for (int i = 4; i < 6; i++) {
                    uint16_t& row = arrays[i][x + (z * size)];

                    if (row == 0)
                        continue;

                    while (row != 0) {
                        uint16_t y0 = std::countr_zero(row);
                        uint16_t height = std::countr_zero(static_cast<uint16_t>(~(row >> y0)));
                        uint16_t mask = ((1u << height) - 1) << y0;

                        uint16_t width {1};
                        row ^= mask;

                        for (std::size_t x0 = (x + 1); x0 < size;x0++)
                        {
                            if ((mask & arrays[i][x0 + (z * size)]) != mask)
                                break;

                            arrays[i][x0 + (z * size)] ^= mask;
                            width++;
                        }

                        std::size_t old_v_size = vertices.size();
                        std::size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 4);
                        indices.resize(old_i_size + 6);

                        uint32_t* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        uint8_t z0f = z;
                        uint8_t x0f = x;
                        uint8_t x1f = (x + width);
                        uint8_t y0f = y0;
                        uint8_t y1f = (y0 + height);

                        uint8_t norm_flip = 0;

                        if (i == 4) {
                            z0f += 1.f;
                            norm_flip = 1;
                        }

                        *vertex_ptr++ = packed_vertex_data((uint8_t)x0f, (uint8_t)y0f, (uint8_t)z0f, norm_flip, 0, 0, 1, 0, 0);
                        *vertex_ptr++ = packed_vertex_data((uint8_t)x0f, (uint8_t)y1f, (uint8_t)z0f, norm_flip, 0, 0, 1, height, 0);
                        *vertex_ptr++ = packed_vertex_data((uint8_t)x1f, (uint8_t)y0f, (uint8_t)z0f, norm_flip, 0, 0, 1, 0, width);
                        *vertex_ptr++ = packed_vertex_data((uint8_t)x1f, (uint8_t)y1f, (uint8_t)z0f, norm_flip, 0, 0, 1, height, width);

                        *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                        *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;

                        triangles += 4;
                    }
                }
            }
        }
        #pragma endregion

        #pragma region front_and_back_faces
        // for (size_t z {0}; z < size; z++)
        // {
        //     for (size_t x {0}; x < size; x++)
        //     {
        //         for (int i {2}; i < 4; i++) {
        //             uint16_t& row = arrays[i][x + (z * size)];
        //
        //             if (row == 0)
        //                 continue;
        //
        //             while (row != 0) {
        //                 uint16_t y0 = std::__countr_zero(row);
        //                 uint16_t height = std::__countr_zero(~(row >> y0));
        //                 uint16_t mask = ((1u << height) - 1) << y0;
        //
        //                 uint16_t width {1};
        //                 row ^= mask;
        //
        //                 for (size_t z0 = (z + 1); z0 < size; z0++)
        //                 {
        //                     if ((mask & arrays[i][x + (z0 * size)]) != mask)
        //                         break;
        //
        //                     arrays[i][x + (z0 * size)] ^= mask;
        //                     width++;
        //                 }
        //
        //                 size_t old_v_size = vertices.size();
        //                 size_t old_i_size = indices.size();
        //                 vertices.resize(old_v_size + 32);
        //                 indices.resize(old_i_size + 6);
        //
        //                 float* vertex_ptr = vertices.data() + old_v_size;
        //                 unsigned int* index_ptr = indices.data() + old_i_size;
        //
        //                 float z0f = (float)z;
        //                 float z1f = (float)(z + width);
        //                 float x0f = (float)x;
        //                 float y0f = (float)y0;
        //                 float y1f = (float)(y0 + height);
        //
        //
        //                 float norm_x = -1;
        //                 float norm_y = 0;
        //                 float norm_z = -1;
        //
        //                 if (i == 2) {
        //                     x0f += 1.f;
        //                     norm_x = 1;
        //                 }
        //
        //                 *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z0f;     *vertex_ptr++ = norm_x;  *vertex_ptr++ = norm_y; *vertex_ptr++ = norm_z;    *vertex_ptr++ = 0.f; *vertex_ptr++ = 0.f;
        //                 *vertex_ptr++ = x0f;  *vertex_ptr++ = y1f; *vertex_ptr++ = z0f;     *vertex_ptr++ = norm_x;  *vertex_ptr++ = norm_y; *vertex_ptr++ = norm_z;    *vertex_ptr++ = height; *vertex_ptr++ = 0.f;
        //                 *vertex_ptr++ = x0f;  *vertex_ptr++ = y0f; *vertex_ptr++ = z1f;     *vertex_ptr++ = norm_x;  *vertex_ptr++ = norm_y; *vertex_ptr++ = norm_z;    *vertex_ptr++ = 0.f; *vertex_ptr++ = width;
        //                 *vertex_ptr++ = x0f;  *vertex_ptr++ = y1f; *vertex_ptr++ = z1f;     *vertex_ptr++ = norm_x;  *vertex_ptr++ = norm_y; *vertex_ptr++ = norm_z;    *vertex_ptr++ = height; *vertex_ptr++ = width;
        //
        //                 *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
        //                 *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;
        //
        //                 triangles += 4;
        //             }
        //         }
        //     }
        // }

        #pragma endregion

        #pragma endregion


        for (std::size_t i {0}; i < size; i++) {
            for (std::size_t j {0}; j < size; j++) {
                for (int k {2}; k < 4; k++) {
                    uint16_t& row = arrays[k][j + (i * size)];

                    if (row == 0)
                        continue;

                    while (row != 0) {
                        uint16_t y0 = std::countr_zero(row);
                        uint16_t height = std::countr_zero(static_cast<uint16_t>(~(row >> y0)));
                        uint16_t mask = ((1u << height) - 1) << y0;

                        uint16_t width {1};
                        row ^= mask;

                        for (std::size_t l = (i + 1); l < size; l++)
                        {
                            if ((mask & arrays[k][j + (l * size)]) != mask)
                                break;

                            arrays[k][j + (l * size)] ^= mask;
                            width++;
                        }

                        std::size_t old_v_size = vertices.size();
                        std::size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 4);
                        indices.resize(old_i_size + 6);

                        uint32_t* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        uint8_t z0f = i;
                        uint8_t z1f = (i + width);
                        uint8_t x0f = j;
                        uint8_t y0f = y0;
                        uint8_t y1f = (y0 + height);

                        uint8_t norm_flip = 0;

                        if (k == 2) {
                            x0f += 1.f;
                            norm_flip = 1;
                        }

                        *vertex_ptr++ = packed_vertex_data((uint8_t)x0f, (uint8_t)y0f, (uint8_t)z0f, norm_flip, 1, 0, 0, 0, 0);
                        *vertex_ptr++ = packed_vertex_data((uint8_t)x0f, (uint8_t)y1f, (uint8_t)z0f, norm_flip, 1, 0, 0, height, 0);
                        *vertex_ptr++ = packed_vertex_data((uint8_t)x0f, (uint8_t)y0f, (uint8_t)z1f, norm_flip, 1, 0, 0, 0, width);
                        *vertex_ptr++ = packed_vertex_data((uint8_t)x0f, (uint8_t)y1f, (uint8_t)z1f, norm_flip, 1, 0, 0, height, width);

                        *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                        *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;

                        triangles += 4;
                    }
                }
            }
        }


        double time_overall = Debug::stop_timer(start);
        double time_greedy = time_overall - time_culling;
        std::cout << "operation took: " << time_overall << " milliseconds\n";
        std::cout << "(face-cull) " << time_culling << " milliseconds (" <<  (time_culling / time_overall) * 100.f << "%)\n";
        std::cout << "(greedy-mesh) " << time_greedy << " milliseconds (" << (time_greedy / time_overall) * 100.f << "%)\n";

        triangles = indices.size();
        std::cout << "triangles: " << triangles / 3 << "\n";
        std::cout << std::endl;

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(uint32_t), vertices.data(), GL_STATIC_DRAW);
        ebo.data(indices.data(), indices.size() * sizeof(unsigned int));

        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(uint32_t), 0);

        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    void Mesh::render() {
        vao.bind();
        glDrawElements(GL_TRIANGLES, triangles, GL_UNSIGNED_INT, 0);
        vao.unbind();
    }
}