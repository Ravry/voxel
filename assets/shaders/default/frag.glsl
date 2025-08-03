#version 330 core

out vec4 color;

uniform vec3 albedo;
uniform sampler2DArray texure_array;

in vec2 oUV;

void main() {
    color = vec4(texture(texure_array, vec3(oUV, 0)).rgb, 1.0);
}