#version 330 core

out vec4 color;

uniform vec3 albedo;

in vec3 pos_ws;

void main() {
    color = vec4(mix((pos_ws.xyz / 8.0), vec3(1), albedo.x) , 1.0);
}