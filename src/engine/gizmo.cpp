#include "gizmo.h"

namespace Voxel::Gizmo {
    void setup_line_box_gizmo(VAO& vao) {
        VBO vbo;
        EBO ebo;
        vao.bind();
        vbo.bind();
        ebo.bind();
        float line_vertices[] {
            0, 0, 0,
            1, 0, 0,
            0, 0, 1,
            1, 0, 1,

            0, 1, 0,
            1, 1, 0,
            0, 1, 1,
            1, 1, 1,
        };
        vbo.data(line_vertices, sizeof(line_vertices));
        unsigned int line_indices[] {
            0, 1,
            1, 3,
            3, 2,
            2, 0,

            4, 5,
            5, 7,
            7, 6,
            6, 4,

            0, 4,
            1, 5,
            2, 6,
            3, 7,
        };
        ebo.data(line_indices, sizeof(line_indices));
        vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    void render_line_box_gizmo(VAO& vao, glm::vec3 position, glm::vec3 size) {
        Shader shader = ResourceManager::get_resource<Shader>("default");
        vao.bind();
        shader.use().set_uniform_mat4("model", glm::scale(glm::translate(glm::mat4(1.0f), position), size)).set_uniform_vec3("albedo", glm::vec3(1.f));
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)0);
        vao.unbind();
    }


    void setup_axis_gizmo(VAO& vao) {
        VBO vbo;
        EBO ebo;
        vao.bind();
        vbo.bind();
        ebo.bind();
        float line_vertices[] {
            0, 0, 0,
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        };
        vbo.data(line_vertices, sizeof(line_vertices));
        unsigned int line_indices[] {
            0, 1,
            0, 2,
            0, 3
        };
        ebo.data(line_indices, sizeof(line_indices));
        vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        vao.unbind();
        vbo.unbind();
        ebo.unbind();
    }

    void render_axis_gizmo(VAO& vao, Camera& camera) {
        glDisable(GL_DEPTH_TEST);
        vao.bind();

        glm::mat4 model = glm::scale(glm::translate(glm::mat4(1), (camera.position + camera.front)), glm::vec3(.05f));

        glLineWidth(4.0f);
        auto& shader = ResourceManager::get_resource<Shader>("default")
            .set_uniform_mat4("model", model)
            .set_uniform_vec3("albedo", glm::vec3(0, 0, 0));

        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)0);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(unsigned)));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(unsigned)));

        glLineWidth(2.0f);
        shader.set_uniform_vec3("albedo", glm::vec3(1, 0, 0));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)0);

        shader.set_uniform_vec3("albedo", glm::vec3(0, 1, 0));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(unsigned)));

        shader.set_uniform_vec3("albedo", glm::vec3(0, 0, 1));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(unsigned)));

        vao.unbind();
        glEnable(GL_DEPTH_TEST);
    }
}
