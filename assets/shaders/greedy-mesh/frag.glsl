#version 430 core

out vec4 color;

uniform vec3 albedo;
uniform sampler2DArray texture_array;

layout(std430, binding = 0) buffer BlockData {
    uint[] blockTypes;
} blockData;

in vec2 oUV;
in vec3 oVertex;
flat in vec3 oNormal;

uniform vec3 light_direction = vec3(.25, -.4, 0);

void main() {
    ivec3 block_coords = ivec3(floor(oVertex - (oNormal * .0001)));
    block_coords = clamp(block_coords, ivec3(0), ivec3(15));
    uint block_type = blockData.blockTypes[block_coords.x + (block_coords.y * 16) + (block_coords.z * 16 * 16)];

    uint diff_faces = block_type & 0x1;
    uint face = uint(dot(normalize(oNormal), vec3(0, 1, 0)) + 1);
    uint index_block_type = (block_type >> 1) + (diff_faces * face);

    vec3 texture_color = texture(texture_array, vec3(oUV, index_block_type)).rgb;

    float diffuse = max(dot(oNormal, -normalize(light_direction)), .4);
    color = vec4(texture_color * diffuse, 1.0);
}