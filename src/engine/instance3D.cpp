#include "instance3D.h"

namespace Voxel {
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