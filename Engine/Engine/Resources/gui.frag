#version 460

struct GuiVStoFs
{
    vec4 color;
    vec2 uv;
};
layout(set = 1, binding = 0) uniform texture2D font;
layout(set = 1, binding = 1) uniform sampler fontSampler;

layout(location = 0) out vec4 outputColor;
layout(location = 0) in GuiVStoFs fsInput;

void main()
{
    vec4 alpha = texture(sampler2D(font, fontSampler), vec2(fsInput.uv));
    outputColor  = fsInput.color * float(alpha.r);
}