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

uniform vec3 light_direction = vec3(0, -1.0, 0);

float shadow_calculation(vec4 _frag_pos) {
    vec3 block_pos = floor(_frag_pos.xyz);
    vec3 face_center = block_pos + vec3(.5) + fs_in.normal * .49;
    vec4 frag_pos = light_space_matrix * vec4(face_center, 1.0);
    vec3 proj_coords = frag_pos.xyz / frag_pos.w;
    proj_coords = proj_coords * .5 + .5;

    if(proj_coords.z > 1.0)
        return 0.0;

    float current_depth = proj_coords.z;
    float bias = .00005;
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    for(int x = 0; x < 1; ++x) {
        for(int y = 0; y < 1; ++y) {
            float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= 1.0;

    return shadow;
}

void main() {
    const float epsilon = 0.5;
    ivec3 block_coords = ivec3(fs_in.vertex - normalize(fs_in.normal) * .01);
    block_coords = clamp(block_coords, ivec3(0), ivec3(15));

    uint block_type = blockData.blockTypes[block_coords.x + (block_coords.y * 16) + (block_coords.z * 16 * 16)];

    uint diff_faces = block_type & 0x1;
    uint face = uint(round(dot(normalize(fs_in.normal), vec3(0, 1, 0)) + 1));
    uint index_block_type = (block_type >> 1) + (diff_faces * face);

    vec3 texture_color = texture(texture_array, vec3(fs_in.uv, index_block_type)).rgb;

    float ambient = 0.3;
    float diffuse = max(dot(fs_in.normal, -normalize(light_direction)), 0.0);
    float shadow = shadow_calculation(fs_in.frag_pos_world_space);
    float lighting = ambient + (1.0 - shadow) * diffuse;

    color = vec4(lighting * texture_color, 1.0);
}