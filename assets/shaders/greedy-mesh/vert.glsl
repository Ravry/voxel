#version 430 core

layout (location = 0) in uint vertex;

layout (std140, binding = 0) uniform Matrices {
    uniform mat4 projection;
    uniform mat4 view;
};

uniform mat4 model;
uniform mat4 light_space_matrix;

out VS_OUT  {
    vec2 uv;
    vec3 vertex;
    vec3 normal;
    vec4 frag_pos_light_space;
} vs_out;

void main() {
    vec3 position_object_space = vec3((vertex >> 27u) & 0x1Fu, (vertex >> 22u) & 0x1Fu, (vertex >> 17u) & 0x1Fu);
    vec4 position_world_space = model * vec4(position_object_space, 1.0);
    gl_Position = projection * view * position_world_space;

    vs_out.vertex = position_object_space;
    vs_out.uv = vec2((vertex >> 12u) & 0x1Fu, (vertex >> 7u) & 0x1Fu);
    int norm_flip = int((vertex >> 6u) & 0x1u);
    vs_out.normal = ((norm_flip * 2) - 1) * vec3((vertex >> 5u) & 0x1u, (vertex >> 4u) & 0x1u, (vertex >> 3u) & 0x1u);
    vs_out.frag_pos_light_space = light_space_matrix * position_world_space;
}
