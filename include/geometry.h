#pragma once

namespace Voxel::Geometry {
    //TODO: fix
    namespace Triangle {
        static float vertices[] {
            -0.5f, -0.5f, 0.f, 0.f, 0.f,
             0.5f, -0.5f, 0.f, 1.f, 0.f,
             -.5f,  0.5f, 0.f, 0.f, 1.f,
             0.5f,  0.5f, 0.f, 1.f, 1.f
        };

        static unsigned int indices[] {
            0, 2, 1,
            1, 2, 3
        };
    }

    namespace Quad {
        static std::vector<float> vertices {
            -1.f, -1.f, 0.f, 0.f, 0.f,
             1.f, -1.f, 0.f, 1.f, 0.f,
            -1.f,  1.f, 0.f, 0.f, 1.f,
             1.f,  1.f, 0.f, 1.f, 1.f
        };

        static std::vector<unsigned int> indices {
            0, 1, 2,
            1, 3, 2
        };
    }

    namespace Cube {
        static std::vector<float> vertices {
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
        };

        static std::vector<unsigned int> indices {
            0, 1, 2,  2, 3, 0,
            1, 5, 6,  6, 2, 1,
            5, 4, 7,  7, 6, 5,
            4, 0, 3,  3, 7, 4,
            3, 2, 6,  6, 7, 3,
            4, 5, 1,  1, 0, 4,
        };

        static std::vector<unsigned int> indices_inside {
            2, 1, 0,  0, 3, 2,
            6, 5, 1,  1, 2, 6,
            7, 4, 5,  5, 6, 7,
            3, 0, 4,  4, 7, 3,
            6, 2, 3,  3, 7, 6,
            1, 5, 4,  4, 0, 1,
        };
    }
}