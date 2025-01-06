#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aColor; // 颜色变量的属性位置值为 1

uniform float time1;
uniform float time2;
uniform float time3;
out vec3 ourColor; // 向片段着色器输出一个颜色

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vec3 rv = vec3(time1, time2, time3);
    vec3 gv = vec3(time2, time3, time1);
    vec3 bv = vec3(time3, time1, time2);
    ourColor = aColor.r * rv + aColor.g * gv + aColor.b * bv; // 将ourColor设置为我们从顶点数据那里得到的输入颜色
}