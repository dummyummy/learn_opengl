#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;
layout (location = 7) in mat3 instanceNormalMatrix;


out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    Normal = mat3(instanceNormalMatrix) * aNormal;
    TexCoords = aTexCoords;
}