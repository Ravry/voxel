#include "instance3D.h"

namespace Voxel {
    Instance3D::Instance3D(
        Mesh* mesh,
        glm::vec3 albedo,
        glm::vec3 position,
        glm::vec3 scale
    ) : Transform(position, glm::vec3(0), scale), mesh(mesh), albedo(albedo) {
        vao.bind();
        vbo.bind();
        ebo.bind();

        vbo.mapped_data(mesh->vertices.data(), mesh->vertices.size() * sizeof(uint32_t));
        ebo.data(mesh->indices.data(), mesh->indices.size() * sizeof(unsigned int), GL_STATIC_DRAW);

        vao.attribi(0, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);

        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    Instance3D::~Instance3D() { }

    void Instance3D::render(Shader& shader) {
        shader
            .use()
            .set_uniform_mat4("model", matrix);

        vao.bind();
        glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
        vao.unbind();
    }
}