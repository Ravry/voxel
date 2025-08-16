#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in vec2 oUV[];
in vec3 oVertex[];
in vec3 oNormal[];

out vec2 fragUV;
out vec3 fragVertex;
out vec3 fragNormal;
out vec3 fragCenter;

void main()
{
    // Compute triangle center
    vec3 center = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0;
    fragCenter = center;

    // Emit the triangle
    for(int i = 0; i < 3; i++)
    {
        fragUV = oUV[i];
        fragVertex = oVertex[i];
        fragNormal = oNormal[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}