#version 460

layout(set = 0, binding = 0) uniform perFrame
{
    vec2 iResolution;
    vec2 iMouse;
    float iTime;
};

struct Position
{
    float x;
    float y; 
    float z;
}

layout(set = 0, binding = 1) buffer PositionStreamBlock
{
    Position positionStream[];
}

layout(set = 0, binding = 1) buffer TriangleBlock
{
    uint8 triangles[];
}

layout(set = 0, binding = 2) buffer TriangleBlock
{
    uint8 triangles[];
}

layout(location = 0) out vec4 outputColor;


void main()
{
    Position p = positionStream[triangles[gl_DrawIDARB]];
	gl_Position = vec4(p.x,p.y,p.z);
}