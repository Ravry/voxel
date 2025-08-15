#version 430 core

layout (location = 0) out vec4 color;

uniform sampler2D main_tex;
uniform vec3 albedo;

uniform bool use_texture;

in vec2 oUV;

void main() {
    color = vec4(use_texture ? vec3(texture(main_tex, oUV).r) : albedo, 1);
}