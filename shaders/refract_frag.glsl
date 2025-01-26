#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform float ior;

void main()
{
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ior);
    FragColor = texture(skybox, R);
}