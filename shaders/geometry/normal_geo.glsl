#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 Normal;
} gs_in[];

uniform float magnitude;

uniform mat4 view;
uniform mat4 projection;

void GenerateLine(mat4 vp, int index)
{
    gl_Position = vp * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = vp * (gl_in[index].gl_Position + vec4(gs_in[index].Normal * magnitude, 0.0));
    EmitVertex();
    EndPrimitive();
}

void main()
{
    mat4 vp = projection * view;

    for (int i = 0; i < 3; i++)
        GenerateLine(vp, i);
}