#version 430 core

layout (location = 0) out vec4 color;

uniform samplerCube main_tex;
uniform vec3 albedo;

in vec3 texture_sample_direction;

void main() {
    color = vec4(texture(main_tex, texture_sample_direction).rgb, 1);
}