#version 430 core

layout (location = 0) in vec3 vertex;

layout (std140, binding = 0) uniform Matrices {
    uniform mat4 projection;
    uniform mat4 view;
};

uniform mat4 view_non_translated;

out vec3 texture_sample_direction;

void main() {
    vec4 position = projection * view_non_translated * vec4(vertex, 1.0);
    gl_Position = position.xyww;
    texture_sample_direction = vertex;
}