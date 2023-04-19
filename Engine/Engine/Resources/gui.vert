#version 460
#extension GL_EXT_shader_8bit_storage: require
#extension GL_ARB_shader_draw_parameters: require
#extension GL_EXT_scalar_block_layout: require

struct ImDrawVert
{
    vec2 position;
    vec2 uv;
    uint color;
};



layout(set = 0, binding = 0, scalar) buffer VertexStreamBlock
{
    ImDrawVert vertexStream[];
};

struct ScaleTranslate{
    vec2 scale;
    vec2 translate;
    uint textureId;
};

layout(push_constant) uniform constants
{
	ScaleTranslate scaleTranslate;
};

struct GuiVStoFs
{
    vec4 color;
    vec2 uv;
};

layout(location = 0) out flat uint textureId;
layout(location = 1) out GuiVStoFs vsOut;

vec4 colorDecode(uint c)
{
    vec4 color;
    color.a = float((c>> 24) & 0xFF);
    color.b = float((c>> 16) & 0xFF);
    color.g = float((c>> 8) & 0xFF);
    color.r = float((c) & 0xFF);
    color /= 255.0;
    return color;
}

void main()
{
    ImDrawVert vertex = vertexStream[gl_VertexIndex];

    vsOut.color = colorDecode(vertex.color);
    vsOut.uv = vertex.uv;
    textureId = scaleTranslate.textureId;
    gl_Position = vec4(vertex.position * scaleTranslate.scale + scaleTranslate.translate, 0.0, 1.0);
}