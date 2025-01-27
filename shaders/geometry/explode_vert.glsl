#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 normalMatrix;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
    vs_out.Normal = mat3(normalMatrix) * aNormal;
    vs_out.TexCoords = aTexCoords;
}