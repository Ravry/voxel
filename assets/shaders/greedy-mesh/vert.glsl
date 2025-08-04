#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 oUV;
out vec3 oVertex;
out vec3 oNormal;

void main() {
    vec4 position_ws = model * vec4(vertex, 1.0);
    gl_Position = projection  * view * position_ws;
    oUV = uv;
    oVertex = vertex;
    oNormal = normal;
}