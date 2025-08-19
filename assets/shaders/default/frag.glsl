#version 430 core

layout (location = 0) out vec4 color;

uniform sampler2D main_tex;
uniform vec3 albedo;

uniform bool use_texture;

in VS_OUT  {
    vec2 uv;
    vec3 normal;
} fs_in;

void main() {
    color = vec4(use_texture ? texture(main_tex, fs_in.uv).rgb : albedo, 1);
}