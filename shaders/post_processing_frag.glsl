#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float offsetScale;  
uniform float offsetFreq;
const int n_octaves = 6;

void main() {
    vec3 col = vec3(0.0);
    float freq = offsetFreq;
    float amp = 1.0f;
    float attenuation = 0.5;
    vec2 offset = vec2(0.0);
    for (int i = 0; i < n_octaves; i++) {
        offset += sin(TexCoords * freq + time * 2.0) * offsetScale * amp;
        amp *= attenuation;
        freq *= 2.0;
    }
    col = vec3(texture(screenTexture, TexCoords + offset));
    FragColor = vec4(col, 1.0);
}