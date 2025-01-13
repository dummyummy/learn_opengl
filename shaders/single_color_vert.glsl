#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;
uniform float width;

void main()
{
    vec3 shiftedPos = aPos + width * aNormal;
    gl_Position = projection * view * model * vec4(shiftedPos, 1.0);
}