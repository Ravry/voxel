#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 pos_ws;

void main() {
    vec4 position_ws = model * vec4(vertex, 1.0);
    pos_ws = position_ws.xyz;
    gl_Position = projection  * view * position_ws;
}