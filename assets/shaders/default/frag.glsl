#version 430 core

out vec4 color;

uniform sampler2D main_tex;
uniform vec3 albedo;

in vec2 oUV;

void main() {
    color = vec4(albedo, 1);
}