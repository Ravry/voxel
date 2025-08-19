#include "engine/instance_3d.h"

namespace Voxel {
    Instance3D::Instance3D(Mesh<float>* mesh, const VAO::AttribInfo& attrib_info, Material* material, glm::vec3 translation) : Transform(translation, glm::vec3(0), glm::vec3(1.f)), mesh(mesh), material(material) {
        vao = std::make_unique<VAO>();
        vbo = std::make_unique<VBO>();
        ebo = std::make_unique<EBO>();

        vao->bind();
        vbo->bind();
        ebo->bind();

        vbo->data(mesh->vertices.data(), mesh->vertices.size() * sizeof(float), GL_STATIC_DRAW);
        ebo->data(mesh->indices.data(), mesh->indices.size() * sizeof(unsigned int), GL_STATIC_DRAW);

        for (const auto& attrib : attrib_info.attribs) {
            vao->attrib(attrib.index, attrib.size, attrib.type, GL_FALSE, attrib_info.stride, attrib.pointer);
        }

        vao->unbind();
        vbo->unbind();
        ebo->unbind();
    }

    void Instance3D::render() {
        material->shader->use()
            .set_uniform_mat4("model", matrix)
            .set_uniform_int("use_texture", material->texture ? 1 : 0);
        if (material->texture) material->texture->bind();
        vao->bind();
        glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
        vao->unbind();
        material->shader->unuse();
    }
}