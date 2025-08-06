#version 330 core

layout (location = 0) in uint vertex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 oUV;
out vec3 oVertex;
flat out vec3 oNormal;

void main() {
    vec3 position = vec3((vertex >> 27u) & 0x1Fu, (vertex >> 22u) & 0x1Fu, (vertex >> 17u) & 0x1Fu);
    gl_Position = projection * view * model * vec4(position, 1.0);
    oVertex = position;

    oUV = vec2((vertex >> 13u) & 0xFu, (vertex >> 9u) & 0xFu);

    int norm_flip = int((vertex >> 8u) & 0x1u);
    oNormal = ((norm_flip * 2) - 1) * vec3((vertex >> 7u) & 0x1u, (vertex >> 6u) & 0x1u, (vertex >> 5u) & 0x1u);
}
