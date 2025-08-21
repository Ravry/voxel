#version 430 core

layout (location = 0) out vec4 color;

uniform vec3 albedo;
uniform sampler2DArray texture_array;
uniform sampler2D shadow_map;

layout(std430, binding = 0) buffer BlockData {
    uint[] blockTypes;
} blockData;

in VS_OUT {
    vec2 uv;
    vec3 vertex;
    vec3 normal;
    vec4 frag_pos_world_space;
} fs_in;

uniform mat4 light_space_matrix;
uniform vec3 light_direction;

float shadow_calculation(vec4 _frag_pos) {
    vec4 frag_pos = light_space_matrix * vec4(_frag_pos.xyz, 1.0);
    vec3 proj_coords = frag_pos.xyz;
    proj_coords = proj_coords * .5 + .5;

    if(proj_coords.z > 1.0)
        return 0.0;

    float current_depth = proj_coords.z;
    float bias = max(0.0005 * (1.0 - dot(fs_in.normal, light_direction)), 0.00005);
    float shadow = 0.0;

    float pcf_depth = texture(shadow_map, proj_coords.xy).r;
    shadow = current_depth - bias > pcf_depth ? 1.0 : 0.0;

    return shadow;
}

 uint access_block_type(int x, int y, int z) {
    int SIZE = 16;
    int SIZE_VALUE_IN_BITS = 8;
    int NUM_VALUES_IN_ONE_UINT = 4;

    int linear_index = x + (y * SIZE) + (z * SIZE * SIZE);
    int block_index = linear_index / NUM_VALUES_IN_ONE_UINT;
    uint area = blockData.blockTypes[block_index];
    int bit_position = (linear_index % NUM_VALUES_IN_ONE_UINT) * SIZE_VALUE_IN_BITS;
    uint block = uint((area >> bit_position) & 0xFF);
    return block;
}

void main() {
    const float epsilon = 0.5;
    ivec3 block_coords = ivec3(fs_in.vertex - normalize(fs_in.normal) * .01);
    block_coords = clamp(block_coords, ivec3(0), ivec3(15));

    uint block_type = access_block_type(block_coords.x, block_coords.y, block_coords.z);

    uint diff_faces = block_type & 0x1;
    uint face = uint(round(dot(normalize(fs_in.normal), vec3(0, 1, 0)) + 1));
    uint index_block_type = (block_type >> 1) + (diff_faces * face);

    vec3 texture_color = texture(texture_array, vec3(fs_in.uv, index_block_type)).rgb;

    float ambient = 0.2;
    float diffuse = max(dot(fs_in.normal, -normalize(light_direction)), 0.0);
    float shadow = shadow_calculation(fs_in.frag_pos_world_space);
    float lighting = ambient + (1.0 - shadow) * diffuse;

    color = vec4(lighting * texture_color, 1.0);
}