#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// uniform sampler2D texture1;

void main()
{             
    // vec4 texColor = texture(texture1, TexCoords);
    // FragColor = texColor;
    FragColor = vec4(0.3, 0.57, 1.0, 0.5);
}