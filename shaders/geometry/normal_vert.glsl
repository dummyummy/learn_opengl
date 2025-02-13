#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 Normal;
} vs_out;

uniform mat4 model;
uniform mat4 normalMatrix;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
    vs_out.Normal = normalize(mat3(normalMatrix) * aNormal);
}