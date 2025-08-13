#version 430 core

out vec4 color;

in vec2 oUV;

uniform sampler2D main_tex;

void main() {
    color = vec4(texture(main_tex, oUV).rgb, 1);
    //color = vec4(oUV, 0, 1);
}
