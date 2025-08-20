#pragma once
#include <bit>
#include <vector>
#include <stdint.h>
#include <string_view>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include "engine/geometry.h"
#include "core/log.h"

namespace Voxel {
    template <typename T> class Mesh {
    public:
        uint32_t packed_vertex_data(
            uint8_t pos_x, uint8_t pos_y, uint8_t pos_z,
            uint8_t flip, uint8_t norm_x, uint8_t norm_y, uint8_t norm_z,
            uint8_t uv_x, uint8_t uv_y
        ) {
            uint32_t packed = 0;
            packed |= (pos_x & 0x1F) << 27;
            packed |= (pos_y & 0x1F) << 22;
            packed |= (pos_z & 0x1F) << 17;

            packed |= (uv_x & 0x1F) << 12;
            packed |= (uv_y & 0x1F) << 7;

            packed |= (flip & 0x1) << 6;
            packed |= (norm_x & 0x1) << 5;
            packed |= (norm_y & 0x1) << 4;
            packed |= (norm_z & 0x1) << 3;

            return packed;
        }

        JPH::Ref<JPH::Shape> create_mesh_collision_shape(std::vector<JPH::Float3>& vertices, std::vector<JPH::uint32>& indices) {
        if (vertices.empty()) return nullptr;

        JPH::MeshShapeSettings meshSettings;
        meshSettings.SetEmbedded();

        for (const auto& vertex : vertices) {
            meshSettings.mTriangleVertices.push_back(vertex);
        }

        for (size_t i = 0; i < indices.size(); i += 3) {
            JPH::IndexedTriangle triangle;
            triangle.mIdx[0] = indices[i];
            triangle.mIdx[1] = indices[i + 1];
            triangle.mIdx[2] = indices[i + 2];
            triangle.mMaterialIndex = 0;
            meshSettings.mIndexedTriangles.push_back(triangle);
        }

        JPH::Shape::ShapeResult result = meshSettings.Create();
        if (result.HasError()) {
            LOG("Failed to create mesh shape: {}", result.GetError().c_str());
            return nullptr;
        }

        return result.Get();
    }

    Mesh(uint16_t* voxels, uint16_t** neighbour_chunk_voxels, const std::size_t size, JPH::Ref<JPH::Shape>& shape)
    {
        #pragma region face_culling
        std::vector<uint16_t> voxels_zy_top_face(size * size, 0);
        std::vector<uint16_t> voxels_zy_bottom_face(size * size, 0);

        std::vector<uint16_t> voxels_xz_right_face(size * size, 0);
        std::vector<uint16_t> voxels_xz_left_face(size * size, 0);

        std::vector<uint16_t> voxels_xz_front_face(size * size, 0);
        std::vector<uint16_t> voxels_xz_back_face(size * size, 0);

        for (std::size_t i {0}; i < size; i++) {
            for (std::size_t j {0}; j < size; j++) {
                //x-axis
                uint16_t row = voxels[j + (i * size)];
                uint32_t row_neighbors_included = static_cast<uint32_t>(row) << 1;

                uint16_t row_x_minus_one_neighbor = neighbour_chunk_voxels[0] ? neighbour_chunk_voxels[0][j + (i * size)] : 0;
                row_neighbors_included |= row_x_minus_one_neighbor >> 15;

                uint16_t row_x_plus_one_neighbor = neighbour_chunk_voxels[1] ? neighbour_chunk_voxels[1][j + (i * size)] : 0;
                row_neighbors_included |= (row_x_plus_one_neighbor & 1) << 17;

                uint16_t row_right_face = static_cast<uint16_t>((row_neighbors_included & ~(row_neighbors_included >> 1)) >> 1);
                uint16_t row_left_face = static_cast<uint16_t>((row_neighbors_included & ~(row_neighbors_included << 1)) >> 1);


                //z-axis
                uint16_t row2 = voxels[j + (i * size) + (size * size)];
                uint32_t row2_neighbors_included = static_cast<uint32_t>(row2) << 1;

                uint16_t row_z_minus_one_neighbor = neighbour_chunk_voxels[2] ? neighbour_chunk_voxels[2][j + (i * size) + (size * size)] : 0;
                row2_neighbors_included |= row_z_minus_one_neighbor >> 15;

                uint16_t row_z_plus_one_neighbor = neighbour_chunk_voxels[3] ? neighbour_chunk_voxels[3][j + (i * size) + (size * size)] : 0;
                row2_neighbors_included |= (row_z_plus_one_neighbor & 1) << 17;

                uint16_t row_front_face = static_cast<uint16_t>((row2_neighbors_included & ~(row2_neighbors_included >> 1)) >> 1);
                uint16_t row_back_face = static_cast<uint16_t>((row2_neighbors_included & ~(row2_neighbors_included << 1)) >> 1);

                //y-axis
                uint16_t row_vertical = voxels[i + (j * size) + (size * size * 2)];
                uint32_t row_vertical_neighbors_included = static_cast<uint32_t>(row_vertical) << 1;

                uint16_t row_y_minus_one_neighbor = neighbour_chunk_voxels[4] ? neighbour_chunk_voxels[4][i + (j * size) + (size * size * 2)] : 0;
                row_vertical_neighbors_included |= row_y_minus_one_neighbor >> 15;

                uint16_t row_y_plus_one_neighbor = neighbour_chunk_voxels[5] ? neighbour_chunk_voxels[5][i + (j * size) + (size * size * 2)] : 0;
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
        #pragma endregion

        uint16_t** arrays = new uint16_t*[6] {
            voxels_zy_top_face.data(), voxels_zy_bottom_face.data(),
            voxels_xz_right_face.data(), voxels_xz_left_face.data(),
            voxels_xz_front_face.data(), voxels_xz_back_face.data(),
        };

        vertices.reserve(size * size * 4 * 6);
        indices.reserve(size * size * 8 * 6);
        unsigned int triangles {0};

        std::vector<JPH::Float3> vertices_jolt;
        std::vector<JPH::uint32> indices_jolt;
        
        #pragma region greedy_meshing

        for (std::size_t i {0}; i < size; i++)
        {
            for (std::size_t j {0}; j < size; j++)
            {
                int index = j + (i * size);
                for (int k {0}; k < 2; k++)
                {
                    uint16_t& row = arrays[k][index];

                    if (row == 0) continue;

                    while (row != 0) {
                        uint16_t x0 = std::countr_zero(static_cast<unsigned>(row));
                        uint16_t height = std::countr_one(static_cast<unsigned>(row >> x0));
                        uint16_t mask = ((1u << height) - 1) << x0;

                        uint16_t width {1};
                        row ^= mask;

                        for (std::size_t l = (j + 1); l < size; l++)
                        {
                            if ((mask & arrays[k][l + (i * size)]) != mask)
                                break;

                            arrays[k][l + (i * size)] ^= mask;
                            width++;
                        }

                        std::size_t old_v_size = vertices.size();
                        std::size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 4);
                        indices.resize(old_i_size + 6);

                        uint32_t* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        uint8_t z0f = j;
                        uint8_t z1f = (j + width);
                        uint8_t x0f = x0;
                        uint8_t x1f = (x0 + height);
                        uint8_t y0f = i;

                        uint8_t norm_flip = 0;
                        if (k == 0) {
                            y0f += 1.f;
                            norm_flip = 1;

                            *index_ptr++ = triangles + 2; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 0;
                            *index_ptr++ = triangles + 2; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 1;
                        }
                        else {
                            *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                            *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;
                        }

                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z0f, norm_flip,  0, 1, 0, 0, 0);
                        *vertex_ptr++ = packed_vertex_data(x1f, y0f, z0f, norm_flip,  0, 1, 0, height, 0);
                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z1f, norm_flip,  0, 1, 0, 0, width);
                        *vertex_ptr++ = packed_vertex_data(x1f, y0f, z1f, norm_flip,  0, 1, 0, height, width);

                        vertices_jolt.push_back(JPH::Float3(x0f, y0f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x1f, y0f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x0f, y0f, z1f));
                        vertices_jolt.push_back(JPH::Float3(x1f, y0f, z1f));

                        triangles += 4;
                    }
                }
                for (int k {2}; k < 4; k++) {
                    uint16_t& row = arrays[k][index];

                    if (row == 0) continue;

                    while (row != 0) {
                        uint16_t y0 = std::countr_zero(static_cast<unsigned>(row));
                        uint16_t height = std::countr_one(static_cast<unsigned>(row >> y0));
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

                        uint8_t x0f = j;
                        uint8_t y0f = y0;
                        uint8_t y1f = (y0 + height);
                        uint8_t z0f = i;
                        uint8_t z1f = (i + width);

                        uint8_t norm_flip = 0;

                        if (k == 2) {
                            x0f += 1.f;
                            norm_flip = 1;

                            *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                            *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;
                        }
                        else {
                            *index_ptr++ = triangles + 2; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 0;
                            *index_ptr++ = triangles + 2; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 1;
                        }

                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z0f, norm_flip, 1, 0, 0, 0, height);
                        *vertex_ptr++ = packed_vertex_data(x0f, y1f, z0f, norm_flip, 1, 0, 0, 0, 0);
                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z1f, norm_flip, 1, 0, 0, width, height);
                        *vertex_ptr++ = packed_vertex_data(x0f, y1f, z1f, norm_flip, 1, 0, 0, width, 0);

                        vertices_jolt.push_back(JPH::Float3(x0f, y0f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x0f, y1f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x0f, y0f, z1f));
                        vertices_jolt.push_back(JPH::Float3(x0f, y1f, z1f));

                        triangles += 4;
                    }
                }
                for (int k {4}; k < 6; k++) {
                    uint16_t& row = arrays[k][index];

                    if (row == 0) continue;

                    while (row != 0) {
                        uint16_t y0 = std::countr_zero(static_cast<unsigned>(row));
                        uint16_t height = std::countr_one(static_cast<unsigned>(row >> y0));
                        uint16_t mask = ((1u << height) - 1) << y0;

                        uint16_t width {1};
                        row ^= mask;

                        for (std::size_t l = (j + 1); l < size; l++)
                        {
                            if ((mask & arrays[k][l + (i * size)]) != mask)
                                break;

                            arrays[k][l + (i * size)] ^= mask;
                            width++;
                        }

                        std::size_t old_v_size = vertices.size();
                        std::size_t old_i_size = indices.size();
                        vertices.resize(old_v_size + 4);
                        indices.resize(old_i_size + 6);

                        uint32_t* vertex_ptr = vertices.data() + old_v_size;
                        unsigned int* index_ptr = indices.data() + old_i_size;

                        uint8_t z0f = i;
                        uint8_t x0f = j;
                        uint8_t x1f = (j + width);
                        uint8_t y0f = y0;
                        uint8_t y1f = (y0 + height);

                        uint8_t norm_flip = 0;

                        if (k == 4) {
                            z0f += 1.f;
                            norm_flip = 1;

                            *index_ptr++ = triangles + 2; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 0;
                            *index_ptr++ = triangles + 2; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 1;
                        }
                        else {
                            *index_ptr++ = triangles + 0; *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 2;
                            *index_ptr++ = triangles + 1; *index_ptr++ = triangles + 3; *index_ptr++ = triangles + 2;
                        }
                        *vertex_ptr++ = packed_vertex_data(x0f, y0f, z0f, norm_flip, 0, 0, 1, 0, height);
                        *vertex_ptr++ = packed_vertex_data(x0f, y1f, z0f, norm_flip, 0, 0, 1, 0, 0);
                        *vertex_ptr++ = packed_vertex_data(x1f, y0f, z0f, norm_flip, 0, 0, 1, width, height);
                        *vertex_ptr++ = packed_vertex_data(x1f, y1f, z0f, norm_flip, 0, 0, 1, width, 0);

                        vertices_jolt.push_back(JPH::Float3(x0f, y0f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x0f, y1f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x1f, y0f, z0f));
                        vertices_jolt.push_back(JPH::Float3(x1f, y1f, z0f));

                        triangles += 4;
                    }
                }
            }
        }

        for (auto _idx : indices) {
            indices_jolt.push_back(_idx);
        }

        #pragma endregion

        shape = (indices.size() > 0) ? create_mesh_collision_shape(vertices_jolt, indices_jolt) : nullptr;
    }
    
        Mesh(const std::vector<T>& vertices, const std::vector<unsigned int>& indices) : vertices(vertices), indices(indices) {}

        std::vector<T> vertices;
        std::vector<unsigned int> indices;
    };
}