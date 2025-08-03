#version 430 core

out vec4 color;

uniform vec3 albedo;
uniform sampler2DArray texure_array;

layout(std430, binding = 0) buffer BlockData {
    uint[512] blockTypes;
} blockData;

in vec2 oUV;
in vec3 blockPos;
in vec3 oNormal;

void main() {
    ivec3 block_coords = ivec3(floor(blockPos));
    block_coords = clamp(block_coords, ivec3(0), ivec3(7));
    uint block_type = blockData.blockTypes[block_coords.x + (block_coords.y * 8) + (block_coords.z * 8 * 8)];
    vec3 texture_color = texture(texure_array, vec3(oUV, block_type)).rgb;
    color = vec4(vec3(block_coords / 8.0), 1.0);
}