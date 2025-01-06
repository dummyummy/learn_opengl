#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 ourColor; // 向片段着色器输出一个颜色
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 transform = projection * view * model;
    gl_Position = transform * vec4(aPos, 1.0);
    ourColor = vec3(1.0, 1.0, 1.0); // 将ourColor设置为我们从顶点数据那里得到的输入颜色
    texCoord = aTexCoord;
}