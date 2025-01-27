#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 Normal;
    vec2 TexCoords;
} gs_in[];

out GS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} gs_out;

uniform mat4 view;
uniform mat4 projection;
uniform float magnitude;

vec3 getNormal()
{
    vec3 e1 = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);
    vec3 e2 = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);
    return normalize(cross(e1, e2));
}

vec4 explode(vec4 position, vec3 normal)
{
    vec3 direction = normal * magnitude;
    return position + vec4(direction, 0.0);
}

void main() {
    vec3 normal = getNormal();

    mat4 vp = projection * view;

    for (int i = 0; i < 3; i++) {
        gs_out.FragPos = vec3(gl_in[i].gl_Position);
        gs_out.Normal = gs_in[i].Normal;
        gs_out.TexCoords = gs_in[i].TexCoords;
        gl_Position = vp * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();

    for (int i = 0; i < 3; i++) {
        gs_out.FragPos = vec3(gl_in[i].gl_Position);
        gs_out.Normal = gs_in[i].Normal;
        gs_out.TexCoords = gs_in[i].TexCoords;
        gl_Position = vp * explode(gl_in[i].gl_Position, normal);
        EmitVertex();
    }
    EndPrimitive();
}