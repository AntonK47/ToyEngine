#version 460

layout(location = 0) out vec4 outputColor;
layout(location=0) in vec3 uv;

void main()
{
    outputColor = vec4(uv, 0.0, 1.0);
}