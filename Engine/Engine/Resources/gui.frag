#version 460
#extension GL_EXT_nonuniform_qualifier : enable
struct GuiVStoFs
{
    vec4 color;
    vec2 uv;
};

layout(set = 1, binding = 0) uniform sampler fontSampler;

layout(set = 2, binding = 0) uniform texture2D textures[];

layout(location = 0) out vec4 outputColor;
layout(location = 0) in flat uint textureId;
layout(location = 1) in GuiVStoFs fsInput;


void main()
{
    vec4 alpha = texture(sampler2D(textures[nonuniformEXT(textureId)], fontSampler), vec2(fsInput.uv));
    if(alpha.g < 0.01 && alpha.b < 0.01)
    {
        outputColor  = fsInput.color * float(alpha.r);
    }
    else
    {
        outputColor = alpha;
    }
}