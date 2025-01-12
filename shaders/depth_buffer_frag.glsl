#version 330 core
out vec4 FragColor;

uniform float near;
uniform float far;

float LinearDepth(float depth)
{
    float z = gl_FragCoord.z * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depth = LinearDepth(gl_FragCoord.z) / (far - near);
    FragColor = vec4(vec3(depth), 1.0);
}