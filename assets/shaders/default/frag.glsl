#version 430 core

out vec4 color;

uniform vec3 albedo;

void main() {
    color = vec4(albedo, 1);
}